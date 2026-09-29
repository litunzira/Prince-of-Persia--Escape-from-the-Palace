#ifndef SOUNDS_H
#define SOUNDS_H

#include "game_globals.h"

#include <mmreg.h>
#include <msacm.h>
#pragma comment(lib, "msacm32.lib")

/* ================================================================== */
/* MP3 PLAYBACK BACKEND                                                */
/*                                                                     */
/* The music and the click/running effects used to be played by        */
/* opening one MCI "mpegvideo" device per sound:                       */
/*                                                                     */
/*     mciSendStringA("open \"gamebgm.mp3\" type mpegvideo alias ...") */
/*                                                                     */
/* That device is a DirectShow graph. On Windows 10/11 the MP3 in it   */
/* is decoded by msmpeg2ac3dec.dll (the Microsoft DTV-DVD Audio        */
/* Decoder), and on this build opening a SECOND such graph while the   */
/* first is still open makes that decoder raise a fail-fast exception  */
/* (0xC0000602) on a quartz.dll worker thread, which kills the process */
/* outright - no exception handler can catch a fail fast. The game     */
/* opened four of them during main(), so it died a few seconds after   */
/* starting, before anything had even been clicked.                    */
/*                                                                     */
/* Opening them one at a time is not an option: the music has to play  */
/* underneath the click and the running loop.                          */
/*                                                                     */
/* So the MP3s are now decoded to PCM once, at load, through the ACM   */
/* codec l3codeca.acm - a completely separate, much older decoder that */
/* has nothing to do with DirectShow - and played through waveOut,     */
/* which is happy to have any number of streams open at once and mixes */
/* them itself.                                                        */
/*                                                                     */
/* Nothing else changes. The same .mp3 files are used, no new files    */
/* are needed, every function below keeps the name, the arguments and  */
/* the behaviour it had, and the volumes are the same (the 500 and 600 */
/* that were passed to "setaudio ... volume to" are now applied to the */
/* samples themselves, on the same 0..1000 scale).                     */
/* ================================================================== */

/* MinGW's older headers lack these; the values are fixed by the SDK.  */
#ifndef MPEGLAYER3_WFX_EXTRA_BYTES
#define MPEGLAYER3_WFX_EXTRA_BYTES    12
#endif
#ifndef MPEGLAYER3_ID_MPEG
#define MPEGLAYER3_ID_MPEG            1
#endif
#ifndef MPEGLAYER3_FLAG_PADDING_OFF
#define MPEGLAYER3_FLAG_PADDING_OFF   0x00000002
#endif
#ifndef ACM_STREAMSIZEF_SOURCE
#define ACM_STREAMSIZEF_SOURCE        0x00000000L
#endif
#ifndef ACM_STREAMCONVERTF_BLOCKALIGN
#define ACM_STREAMCONVERTF_BLOCKALIGN 0x00000004
#endif
#ifndef ACM_STREAMCONVERTF_START
#define ACM_STREAMCONVERTF_START      0x00000010
#endif
#ifndef ACM_STREAMCONVERTF_END
#define ACM_STREAMCONVERTF_END        0x00000020
#endif

struct Mp3FrameInfo { int br, sr, ch, len; };

static const int mp3RatesV1[16] = { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 };
static const int mp3RatesV2[16] = { 0,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160, 0 };
static const int mp3SampleRates[4][3] = {
	{ 11025, 12000, 8000 }, { 0, 0, 0 }, { 22050, 24000, 16000 }, { 44100, 48000, 32000 }
};

// Reads one MPEG audio frame header. Returns 0 when `p` is not one.
static int mp3ParseFrame(const unsigned char *p, long avail, Mp3FrameInfo *f){
	int ver, layer, brIdx, srIdx, pad, mode, br, sr, spf;

	if (avail < 4) return 0;
	if (p[0] != 0xFF || (p[1] & 0xE0) != 0xE0) return 0;

	ver   = (p[1] >> 3) & 3;
	layer = (p[1] >> 1) & 3;
	brIdx = (p[2] >> 4) & 0xF;
	srIdx = (p[2] >> 2) & 3;
	pad   = (p[2] >> 1) & 1;
	mode  = (p[3] >> 6) & 3;

	if (ver == 1 || layer != 1) return 0;               // layer==1 is Layer III
	if (brIdx == 0 || brIdx == 15 || srIdx == 3) return 0;

	br = (ver == 3) ? mp3RatesV1[brIdx] : mp3RatesV2[brIdx];
	sr = mp3SampleRates[ver][srIdx];
	if (br == 0 || sr == 0) return 0;

	spf = (ver == 3) ? 1152 : 576;
	f->br  = br;
	f->sr  = sr;
	f->ch  = (mode == 3) ? 1 : 2;
	f->len = (spf / 8) * br * 1000 / sr + pad;
	return (f->len > 4 && f->len <= avail) ? 1 : 0;
}

// Finds the first real frame. A single 0xFF 0xE? pair turns up by chance
// inside compressed audio all the time, so a candidate only counts when
// four frames chain from it, each landing exactly on the next header.
static long mp3FindStart(const unsigned char *b, long n, Mp3FrameInfo *first){
	long off;
	for (off = 0; off + 4 < n; off++){
		Mp3FrameInfo f;
		long p = off;
		int i;
		for (i = 0; i < 4; i++){
			if (!mp3ParseFrame(b + p, n - p, &f)) break;
			p += f.len;
			if (p >= n){ i++; break; }
		}
		if (i >= 4){
			mp3ParseFrame(b + off, n - off, first);
			return off;
		}
	}
	return -1;
}

// Decodes `fileName` to 16-bit PCM, scaling every sample by
// volumePermille/1000 so the result is already at playing volume.
// Returns 1 on success; on any failure the sound is simply left
// unloaded and stays silent, exactly as a missing file always has.
static int mp3DecodeToPcm(const char *fileName, std::vector<char> &pcm,
                          WAVEFORMATEX *fmt, int volumePermille){
	std::vector<char> raw;
	Mp3FrameInfo f;
	MPEGLAYER3WAVEFORMAT src;
	HACMSTREAM hs = NULL;
	ACMSTREAMHEADER sh;
	DWORD need = 0;
	long start;

	{
		std::ifstream file(fileName, std::ios::binary | std::ios::ate);
		std::streamsize size;
		if (!file.is_open()) return 0;
		size = file.tellg();
		if (size <= 0) return 0;
		file.seekg(0, std::ios::beg);
		raw.resize((size_t)size);
		if (!file.read(&raw[0], size)) return 0;
	}

	start = mp3FindStart((const unsigned char *)&raw[0], (long)raw.size(), &f);
	if (start < 0) return 0;

	ZeroMemory(&src, sizeof(src));
	src.wfx.wFormatTag      = WAVE_FORMAT_MPEGLAYER3;
	src.wfx.nChannels       = (WORD)f.ch;
	src.wfx.nSamplesPerSec  = f.sr;
	src.wfx.nAvgBytesPerSec = f.br * 1000 / 8;
	src.wfx.nBlockAlign     = 1;
	src.wfx.wBitsPerSample  = 0;
	src.wfx.cbSize          = MPEGLAYER3_WFX_EXTRA_BYTES;
	src.wID                 = MPEGLAYER3_ID_MPEG;
	src.fdwFlags            = MPEGLAYER3_FLAG_PADDING_OFF;
	src.nBlockSize          = (WORD)f.len;
	src.nFramesPerBlock     = 1;
	src.nCodecDelay         = 1393;

	ZeroMemory(fmt, sizeof(*fmt));
	fmt->wFormatTag      = WAVE_FORMAT_PCM;
	fmt->nChannels       = (WORD)f.ch;
	fmt->nSamplesPerSec  = f.sr;
	fmt->wBitsPerSample  = 16;
	fmt->nBlockAlign     = (WORD)(f.ch * 2);
	fmt->nAvgBytesPerSec = f.sr * f.ch * 2;
	fmt->cbSize          = 0;

	if (acmStreamOpen(&hs, NULL, (WAVEFORMATEX *)&src, fmt, NULL, 0, 0, 0) != 0)
		return 0;

	if (acmStreamSize(hs, (DWORD)(raw.size() - start), &need, ACM_STREAMSIZEF_SOURCE) != 0 || need == 0){
		acmStreamClose(hs, 0);
		return 0;
	}
	need += f.sr * f.ch * 2;                            // slack for codec delay
	pcm.resize(need);

	ZeroMemory(&sh, sizeof(sh));
	sh.cbStruct    = sizeof(sh);
	sh.pbSrc       = (LPBYTE)&raw[0] + start;
	sh.cbSrcLength = (DWORD)(raw.size() - start);
	sh.pbDst       = (LPBYTE)&pcm[0];
	sh.cbDstLength = need;

	if (acmStreamPrepareHeader(hs, &sh, 0) != 0){
		acmStreamClose(hs, 0);
		pcm.clear();
		return 0;
	}
	if (acmStreamConvert(hs, &sh, ACM_STREAMCONVERTF_BLOCKALIGN |
	                              ACM_STREAMCONVERTF_START |
	                              ACM_STREAMCONVERTF_END) != 0 || sh.cbDstLengthUsed == 0){
		acmStreamUnprepareHeader(hs, &sh, 0);
		acmStreamClose(hs, 0);
		pcm.clear();
		return 0;
	}

	pcm.resize(sh.cbDstLengthUsed);
	acmStreamUnprepareHeader(hs, &sh, 0);
	acmStreamClose(hs, 0);

	// Same 0..1000 volume scale MCI's "setaudio volume to N" used.
	if (volumePermille >= 0 && volumePermille < 1000){
		short *s = (short *)&pcm[0];
		size_t i, count = pcm.size() / 2;
		for (i = 0; i < count; i++)
			s[i] = (short)((int)s[i] * volumePermille / 1000);
	}
	return 1;
}

/* ---------------------------------------------------------------- */
/* One decoded sound on its own waveOut stream.                       */
/* ---------------------------------------------------------------- */
struct WaveVoice {
	HWAVEOUT     hwo;
	WAVEHDR      hdr;
	WAVEFORMATEX fmt;
	std::vector<char> pcm;
	int ready;
};

// waveOutWrite loops the buffer dwLoops times; this is "for as long as
// the level lasts" without ever needing a callback to restart it.
#define WAVE_VOICE_FOREVER 0x0FFFFFFF

static void waveVoiceZero(WaveVoice *v){
	v->hwo = NULL;
	v->ready = 0;
	ZeroMemory(&v->hdr, sizeof(v->hdr));
	ZeroMemory(&v->fmt, sizeof(v->fmt));
}

// Decodes the MP3 and opens the output stream. Returns 1 on success.
// Failure leaves the voice silent rather than stopping the game.
static int waveVoiceLoad(WaveVoice *v, const char *mp3File, int volumePermille){
	if (v->ready) return 1;
	waveVoiceZero(v);

	if (!mp3DecodeToPcm(mp3File, v->pcm, &v->fmt, volumePermille))
		return 0;

	if (waveOutOpen(&v->hwo, WAVE_MAPPER, &v->fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR){
		v->hwo = NULL;
		v->pcm.clear();
		return 0;
	}

	ZeroMemory(&v->hdr, sizeof(v->hdr));
	v->hdr.lpData         = &v->pcm[0];
	v->hdr.dwBufferLength = (DWORD)v->pcm.size();

	if (waveOutPrepareHeader(v->hwo, &v->hdr, sizeof(v->hdr)) != MMSYSERR_NOERROR){
		waveOutClose(v->hwo);
		v->hwo = NULL;
		v->pcm.clear();
		return 0;
	}

	v->ready = 1;
	return 1;
}

// Restarts the sound from the beginning, looping it if `loop` is set.
static void waveVoicePlay(WaveVoice *v, int loop){
	if (!v->ready) return;

	waveOutReset(v->hwo);                      // drop whatever is queued

	v->hdr.dwFlags      &= ~WHDR_DONE;
	v->hdr.dwLoops       = loop ? WAVE_VOICE_FOREVER : 1;
	if (loop) v->hdr.dwFlags |= (WHDR_BEGINLOOP | WHDR_ENDLOOP);
	else      v->hdr.dwFlags &= ~(WHDR_BEGINLOOP | WHDR_ENDLOOP);

	waveOutWrite(v->hwo, &v->hdr, sizeof(v->hdr));
}

static void waveVoiceStop(WaveVoice *v){
	if (!v->ready) return;
	waveOutReset(v->hwo);
}

static void waveVoiceClose(WaveVoice *v){
	if (!v->ready) return;
	waveOutReset(v->hwo);
	waveOutUnprepareHeader(v->hwo, &v->hdr, sizeof(v->hdr));
	waveOutClose(v->hwo);
	v->pcm.clear();
	waveVoiceZero(v);
}

std::vector<char> attackSoundData;
int attackSoundLoaded = 0;

// Load the short attack sound into RAM once at startup.
// Playing from memory avoids MP3 decoding and disk/file-system work when E is pressed.
void initAttackSound(){
	std::ifstream file("attack.wav", std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return;

	std::streamsize size = file.tellg();
	if (size <= 0)
		return;

	file.seekg(0, std::ios::beg);
	attackSoundData.resize((size_t)size);

	if (file.read(attackSoundData.data(), size))
		attackSoundLoaded = 1;
	else
		attackSoundData.clear();
}

void playAttackSound(){
	if (!attackSoundLoaded || attackSoundData.empty())
		return;

	// SND_MEMORY + SND_ASYNC: play the already-loaded WAV without blocking
	// the game loop. No MP3 decoding, seek, stop, or disk access occurs here.
	PlaySoundA(
		attackSoundData.data(),
		NULL,
		SND_MEMORY | SND_ASYNC | SND_NODEFAULT
		);
}


/* ---------------------------------------------------------------- */
/* Button click sound                                                  */
/* ---------------------------------------------------------------- */
int buttonSoundOpened = 0;
WaveVoice buttonVoice;

void initButtonSound(){
	if (buttonSoundOpened) return;

	// Volume 500/1000 = 50%, as before.
	if (waveVoiceLoad(&buttonVoice, "click.mp3", 500))
		buttonSoundOpened = 1;
}

void playButtonSound(){
	if (!buttonSoundOpened)
		initButtonSound();

	if (!buttonSoundOpened) return;

	waveVoicePlay(&buttonVoice, 0);            // one shot, from the start
}

void closeButtonSound(){
	if (buttonSoundOpened){
		waveVoiceClose(&buttonVoice);
		buttonSoundOpened = 0;
	}
}

/* ---------------------------------------------------------------- */
/* Running sound                                                      */
/* ---------------------------------------------------------------- */
int runningSoundOpened = 0;
int runningSoundPlaying = 0;
WaveVoice runningVoice;

// Open the running sound once. Volume is 500/1000 = 50%.
void initRunningSound(){
	if (runningSoundOpened) return;

	if (waveVoiceLoad(&runningVoice, "running.mp3", 500))
		runningSoundOpened = 1;
}

void startRunningSound(){
	if (!runningSoundOpened)
		initRunningSound();

	if (!runningSoundOpened || runningSoundPlaying) return;

	waveVoicePlay(&runningVoice, 1);           // looped, as "play ... repeat" was
	runningSoundPlaying = 1;
}

void stopRunningSound(){
	if (!runningSoundOpened || !runningSoundPlaying) return;

	waveVoiceStop(&runningVoice);
	runningSoundPlaying = 0;
}

void closeRunningSound(){
	if (runningSoundOpened){
		waveVoiceClose(&runningVoice);
		runningSoundOpened = 0;
		runningSoundPlaying = 0;
	}
}

/* ---------------------------------------------------------------- */
/* Background music                                                   */
/* ---------------------------------------------------------------- */
int gameBgmOpened = 0;
int gameBgmPlaying = 0;
WaveVoice gameBgmVoice;

// Open the background music once. The actual playback starts when
// START GAME is clicked. Volume is 500/1000 = 50%.
void initGameBGM(){
	if (gameBgmOpened) return;

	if (waveVoiceLoad(&gameBgmVoice, "gamebgm.mp3", 500))
		gameBgmOpened = 1;
}

void startGameBGM(){
	if (!gameBgmOpened)
		initGameBGM();

	if (!gameBgmOpened) return;

	waveVoicePlay(&gameBgmVoice, 1);           // looped, from the start
	gameBgmPlaying = 1;
}

void stopGameBGM(){
	if (!gameBgmOpened) return;

	waveVoiceStop(&gameBgmVoice);
	gameBgmPlaying = 0;
}

void closeGameBGM(){
	if (gameBgmOpened){
		waveVoiceClose(&gameBgmVoice);
		gameBgmOpened = 0;
		gameBgmPlaying = 0;
	}
}

/* ================================================================== */
/* LEVEL 3 AUDIO                                                       */
/*                                                                     */
/* Two additions, both built on machinery that is already in this      */
/* file so nothing about the existing audio behaviour changes:         */
/*                                                                     */
/* 1) The final victory music, played exactly like gamebgm.mp3 above   */
/*    (one voice, loaded at most once, guarded by its own ...Opened    */
/*    flag, closed alongside the others on EXIT). It uses wonbgm.mp3,  */
/*    which already ships with the project.                            */
/*                                                                     */
/* 2) A handful of short Level 3 effect sounds, loaded into RAM and    */
/*    played with PlaySoundA(SND_MEMORY | SND_ASYNC) - the exact same  */
/*    approach attack.wav already uses, so there is no MCI handle to   */
/*    leak, no decoding on the game thread, and no way to open the     */
/*    same sound twice.                                                */
/*                                                                     */
/*    EVERY ONE OF THESE FILES IS OPTIONAL. loadMemorySound() simply   */
/*    reports failure when the file is not next to the executable, the */
/*    matching play...() call then does nothing at all, and Level 3    */
/*    runs perfectly (just quieter). Drop any of these 24-bit/16-bit   */
/*    PCM .wav files in beside attack.wav to enable them:              */
/*        collapse_warn.wav   - a slab starts to crack                 */
/*        collapse.wav        - a slab drops away                      */
/*        water_rise.wav      - the flood surges to a new level        */
/*        guardian_hit.wav    - the Guardian takes a sword blow        */
/*        guardian_death.wav  - the Guardian falls                     */
/*        escape.wav          - the palace begins its final collapse   */
/*        bat.wav             - a Level 2 bat sweeps in                */
/*        bat_hit.wav         - a Level 2 bat connects                 */
/*        heart.wav           - a health heart is collected            */
/*        bite.wav            - a Level 3 piranha takes hold           */
/*        climb.wav           - hauling out of Level 3's water ('R')   */
/*    (The Guardian's own swing deliberately reuses the existing       */
/*    attack.wav through playAttackSound(), so it is never silent.)    */
/* ================================================================== */

/* ---------------------------------------------------------------- */
/* Final victory music (wonbgm.mp3)                                   */
/* ---------------------------------------------------------------- */
int victoryBgmOpened = 0;
int victoryBgmPlaying = 0;
WaveVoice victoryBgmVoice;

void initVictoryBGM(){
	if (victoryBgmOpened) return;

	// Volume 600/1000 = 60%, as before.
	if (waveVoiceLoad(&victoryBgmVoice, "wonbgm.mp3", 600))
		victoryBgmOpened = 1;
}

void startVictoryBGM(){
	if (!victoryBgmOpened)
		initVictoryBGM();

	if (!victoryBgmOpened) return;

	waveVoicePlay(&victoryBgmVoice, 1);        // looped, from the start
	victoryBgmPlaying = 1;
}

void stopVictoryBGM(){
	if (!victoryBgmOpened) return;

	waveVoiceStop(&victoryBgmVoice);
	victoryBgmPlaying = 0;
}

void closeVictoryBGM(){
	if (victoryBgmOpened){
		waveVoiceClose(&victoryBgmVoice);
		victoryBgmOpened = 0;
		victoryBgmPlaying = 0;
	}
}

/* ---------------------------------------------------------------- */
/* Optional in-memory Level 3 effect sounds                           */
/* ---------------------------------------------------------------- */
// Returns 1 only if the whole file was read successfully. A missing
// file is NOT an error here - it just leaves the sound unloaded.
int loadMemorySound(const char *fileName, std::vector<char> &out){
	std::ifstream file(fileName, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return 0;

	std::streamsize size = file.tellg();
	if (size <= 0)
		return 0;

	file.seekg(0, std::ios::beg);
	out.resize((size_t)size);

	if (file.read(out.data(), size))
		return 1;

	out.clear();
	return 0;
}

void playMemorySound(std::vector<char> &data, int loaded){
	if (!loaded || data.empty())
		return;

	PlaySoundA(
		data.data(),
		NULL,
		SND_MEMORY | SND_ASYNC | SND_NODEFAULT
		);
}

std::vector<char> level3CollapseWarnData;   int level3CollapseWarnLoaded = 0;
std::vector<char> level3CollapseData;       int level3CollapseLoaded = 0;
std::vector<char> level3WaterRiseData;      int level3WaterRiseLoaded = 0;
std::vector<char> level3GuardianHitData;    int level3GuardianHitLoaded = 0;
std::vector<char> level3GuardianDeathData;  int level3GuardianDeathLoaded = 0;
std::vector<char> level3EscapeData;         int level3EscapeLoaded = 0;

/* Level 2's bats use the same optional-file mechanism - bat.wav as one
 * sweeps in, bat_hit.wav when one connects. Both missing is fine; the
 * bats just fly silently. */
std::vector<char> level2BatData;            int level2BatLoaded = 0;
std::vector<char> level2BatHitData;         int level2BatHitLoaded = 0;

/* Health hearts (Levels 2 and 3) - heart.wav when one is taken. */
std::vector<char> heartData;                int heartLoaded = 0;

/* Level 3's water - a piranha taking hold, and pulling out onto a ledge. */
std::vector<char> level3BiteData;           int level3BiteLoaded = 0;
std::vector<char> level3ClimbData;          int level3ClimbLoaded = 0;

int level3SoundsInitialised = 0;

// Called once from main(), before the first frame. Guarded so it can
// never load the same buffers twice even if it is called again.
void initLevel3Sounds(){
	if (level3SoundsInitialised) return;
	level3SoundsInitialised = 1;

	level3CollapseWarnLoaded   = loadMemorySound("collapse_warn.wav",  level3CollapseWarnData);
	level3CollapseLoaded       = loadMemorySound("collapse.wav",       level3CollapseData);
	level3WaterRiseLoaded      = loadMemorySound("water_rise.wav",     level3WaterRiseData);
	level3GuardianHitLoaded    = loadMemorySound("guardian_hit.wav",   level3GuardianHitData);
	level3GuardianDeathLoaded  = loadMemorySound("guardian_death.wav", level3GuardianDeathData);
	level3EscapeLoaded         = loadMemorySound("escape.wav",         level3EscapeData);

	/* Level 2's bats - optional in exactly the same way. */
	level2BatLoaded            = loadMemorySound("bat.wav",            level2BatData);
	level2BatHitLoaded         = loadMemorySound("bat_hit.wav",        level2BatHitData);
	heartLoaded                = loadMemorySound("heart.wav",          heartData);
	level3BiteLoaded           = loadMemorySound("bite.wav",           level3BiteData);
	level3ClimbLoaded          = loadMemorySound("climb.wav",          level3ClimbData);
}

void playCollapseWarnSound(){   playMemorySound(level3CollapseWarnData,  level3CollapseWarnLoaded); }
void playCollapseSound(){       playMemorySound(level3CollapseData,      level3CollapseLoaded); }
void playWaterRiseSound(){      playMemorySound(level3WaterRiseData,     level3WaterRiseLoaded); }
void playGuardianHitSound(){    playMemorySound(level3GuardianHitData,   level3GuardianHitLoaded); }
void playGuardianDeathSound(){  playMemorySound(level3GuardianDeathData, level3GuardianDeathLoaded); }
void playEscapeSound(){         playMemorySound(level3EscapeData,        level3EscapeLoaded); }
void playBatSound(){            playMemorySound(level2BatData,           level2BatLoaded); }
void playBatHitSound(){         playMemorySound(level2BatHitData,        level2BatHitLoaded); }
void playHeartSound(){          playMemorySound(heartData,               heartLoaded); }
void playBiteSound(){           playMemorySound(level3BiteData,          level3BiteLoaded); }
void playClimbSound(){          playMemorySound(level3ClimbData,         level3ClimbLoaded); }

#endif
