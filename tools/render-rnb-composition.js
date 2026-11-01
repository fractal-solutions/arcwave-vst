#!/usr/bin/env node
"use strict";

const fs = require("fs");
const path = require("path");

const ROOT = path.resolve(__dirname, "..");
const WT_DIR = path.join(ROOT, "assets", "wavetables");
const OUT_DIR = path.join(ROOT, "out");
const OUT_NAME = "rnb-composition.wav";

const SR = 48000;
const BPM = 76;
const BEAT = 60 / BPM;
const BAR = BEAT * 4;
const BARS = 20;
const TAIL = 5;
const TOTAL = BARS * BAR + TAIL;
const N = Math.ceil(TOTAL * SR);

const CHORDS = {
  Ebmaj9: { notes: [51, 58, 62, 65, 67], bass: 39 },
  Cm9: { notes: [48, 55, 60, 62, 63], bass: 36 },
  Abmaj9: { notes: [44, 51, 55, 60, 62], bass: 44 },
  Bb13: { notes: [46, 53, 56, 62, 67], bass: 46 },
  Fm9: { notes: [41, 55, 56, 60, 63], bass: 41 },
  Dbmaj9: { notes: [49, 56, 60, 63, 65], bass: 37 },
  Bbm9: { notes: [46, 53, 56, 60, 61], bass: 46 },
  C7b9: { notes: [48, 55, 58, 61, 64], bass: 36 },
};

const PROGRESSION = [
  "Ebmaj9", "Cm9", "Abmaj9", "Bb13",
  "Ebmaj9", "Cm9", "Abmaj9", "Bb13",
  "Ebmaj9", "Cm9", "Abmaj9", "Bb13",
  "Fm9", "Dbmaj9", "Bbm9", "C7b9",
  "Ebmaj9", "Cm9", "Abmaj9", "Ebmaj9",
];

let tables;

function mulberry32(seed) {
  let a = seed >>> 0;
  return function () {
    a = (a + 0x6d2b79f5) | 0;
    let t = Math.imul(a ^ (a >>> 15), 1 | a);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

const rand = mulberry32(0x5eed1234);

function jitter(amount) {
  return (rand() * 2 - 1) * amount;
}

function clamp(x, lo, hi) {
  return x < lo ? lo : x > hi ? hi : x;
}

function midiToHz(n) {
  return 440 * Math.pow(2, (n - 69) / 12);
}

function at(bar, beat) {
  return bar * BAR + beat * BEAT;
}

function resampleCycle(src, size) {
  const out = new Float32Array(size + 1);
  const n = src.length;
  for (let i = 0; i < size; i++) {
    const x = (i / size) * n;
    const i0 = Math.floor(x) % n;
    const i1 = (i0 + 1) % n;
    const t = x - Math.floor(x);
    out[i] = src[i0] * (1 - t) + src[i1] * t;
  }
  out[size] = out[0];
  return out;
}

function loadTables() {
  const raw = fs.readFileSync(path.join(WT_DIR, "index.json"), "utf8").replace(/^\uFEFF/, "");
  const idx = JSON.parse(raw);
  const map = new Map();
  for (const w of idx.wavetables) {
    const buf = fs.readFileSync(path.join(WT_DIR, w.raw_f32));
    const src = new Float32Array(w.samples);
    for (let i = 0; i < w.samples; i++) src[i] = buf.readFloatLE(i * 4);
    map.set(w.name, resampleCycle(src, 2048));
  }
  return map;
}

function readTable(table, phase) {
  const p = phase - Math.floor(phase);
  const x = p * (table.length - 1);
  const i = x | 0;
  const t = x - i;
  return table[i] * (1 - t) + table[i + 1] * t;
}

function adsr(t, dur, a, d, s, r) {
  if (t < 0) return 0;
  if (t < a) return t / a;
  if (t < a + d) return 1 - (1 - s) * ((t - a) / d);
  if (t < dur) return s;
  const rt = t - dur;
  if (rt >= r) return 0;
  const x = 1 - rt / r;
  return s * x * x;
}

class SVF {
  constructor() {
    this.ic1 = 0;
    this.ic2 = 0;
  }
  process(x, g, k) {
    const a1 = 1 / (1 + g * (g + k));
    const a2 = g * a1;
    const a3 = g * a2;
    const v3 = x - this.ic2;
    const v1 = a1 * this.ic1 + a2 * v3;
    const v2 = this.ic2 + a2 * this.ic1 + a3 * v3;
    this.ic1 = 2 * v1 - this.ic1;
    this.ic2 = 2 * v2 - this.ic2;
    return v2;
  }
}

function bus() {
  return { l: new Float32Array(N), r: new Float32Array(N) };
}

function sendsObject() {
  return { rev: bus(), del: bus() };
}

function renderVoice(target, sends, spec) {
  const table = spec.table;
  if (!table) return 0;

  const start = spec.start;
  const dur = spec.dur;
  const release = spec.release != null ? spec.release : 0.4;
  const span = dur + release + 0.05;
  const i0 = Math.max(0, Math.floor(start * SR));
  const i1 = Math.min(N, Math.ceil((start + span) * SR));
  if (i1 <= i0) return 0;

  const unison = spec.unison || 1;
  const detune = spec.detune || 0;
  const stereo = spec.stereo || 0;
  const pan = spec.pan || 0;
  const gain = (spec.gain != null ? spec.gain : 1) / Math.sqrt(unison);
  const velocity = spec.velocity != null ? spec.velocity : 1;
  const attack = spec.attack != null ? spec.attack : 0.005;
  const decay = spec.decay != null ? spec.decay : 0.3;
  const sustain = spec.sustain != null ? spec.sustain : 0.7;
  const cutoff = spec.cutoff != null ? spec.cutoff : 2000;
  const q = spec.q != null ? spec.q : 0.9;
  const filterEnv = spec.filterEnv || 0;
  const filterAttack = spec.filterAttack != null ? spec.filterAttack : 0.005;
  const filterDecay = spec.filterDecay != null ? spec.filterDecay : 0.3;
  const lfoRate = spec.lfoRate || 0;
  const lfoAmount = spec.lfoAmount || 0;
  const vibratoRate = spec.vibratoRate || 0;
  const vibratoCents = spec.vibratoCents || 0;
  const sendRev = spec.sendRev || 0;
  const sendDel = spec.sendDel || 0;
  const glideFrom = spec.glideFrom;
  const glideTime = spec.glideTime != null ? spec.glideTime : 0.06;

  const baseHz = midiToHz(spec.note);
  const fromHz = glideFrom != null ? midiToHz(glideFrom) : baseHz;
  const k = 1 / q;

  const ratios = new Float32Array(unison);
  const phases = new Float32Array(unison);
  for (let u = 0; u < unison; u++) {
    const spread = unison > 1 ? (u / (unison - 1)) * 2 - 1 : 0;
    ratios[u] = Math.pow(2, (detune * spread) / 1200);
    phases[u] = spec.randomPhase ? rand() : 0;
  }

  const cutoffConstant = filterEnv === 0 && lfoAmount === 0;
  const gConstant = cutoffConstant
    ? (Math.PI * clamp(cutoff, 20, SR * 0.45)) / SR
    : 0;

  const filterL = new SVF();
  const filterR = new SVF();
  let peak = 0;

  for (let i = i0; i < i1; i++) {
    const t = i / SR - start;
    const env = adsr(t, dur, attack, decay, sustain, release);
    if (env <= 0) continue;

    let g;
    if (cutoffConstant) {
      g = gConstant;
    } else {
      const fenv = filterEnv !== 0 ? adsr(t, dur, filterAttack, filterDecay, 0, release) : 0;
      const lfo = lfoAmount !== 0 ? Math.sin(2 * Math.PI * lfoRate * t) * lfoAmount : 0;
      const fc = clamp(cutoff * Math.pow(2, filterEnv * fenv + lfo), 20, SR * 0.45);
      g = (Math.PI * fc) / SR;
    }

    let fundamental = baseHz;
    if (glideFrom != null && t < glideTime) {
      fundamental = fromHz * Math.pow(baseHz / fromHz, t / glideTime);
    }
    if (vibratoCents !== 0) {
      fundamental *= Math.pow(2, (vibratoCents * Math.sin(2 * Math.PI * vibratoRate * t)) / 1200);
    }

    let l = 0;
    let r = 0;
    for (let u = 0; u < unison; u++) {
      const spread = unison > 1 ? (u / (unison - 1)) * 2 - 1 : 0;
      const hz = fundamental * ratios[u];
      const sample = readTable(table, phases[u]) * gain * velocity;
      const p = clamp(pan + spread * stereo, -1, 1);
      l += sample * Math.sqrt((1 - p) / 2);
      r += sample * Math.sqrt((1 + p) / 2);
      phases[u] += hz / SR;
      phases[u] -= Math.floor(phases[u]);
    }

    l = filterL.process(l, g, k) * env;
    r = filterR.process(r, g, k) * env;

    target.l[i] += l;
    target.r[i] += r;

    if (sendRev !== 0) {
      sends.rev.l[i] += l * sendRev;
      sends.rev.r[i] += r * sendRev;
    }
    if (sendDel !== 0) {
      sends.del.l[i] += l * sendDel;
      sends.del.r[i] += r * sendDel;
    }

    const mag = Math.abs(l) + Math.abs(r);
    if (mag > peak) peak = mag;
  }
  return peak;
}

function section(bar) {
  if (bar < 4) return "intro";
  if (bar < 12) return "main";
  if (bar < 16) return "bridge";
  return "outro";
}

function scheduleKeys(target, sends) {
  for (let bar = 0; bar < BARS; bar++) {
    const chord = CHORDS[PROGRESSION[bar]];
    const sec = section(bar);
    const chordNotes = sec === "intro" ? chord.notes : chord.notes.slice(1);
    let hits;

    if (sec === "intro") {
      hits = [{ beat: 0, dur: 3.4, gain: 0.5 }];
    } else if (sec === "main") {
      hits = [
        { beat: 0, dur: 1.7, gain: 0.78 },
        { beat: 2.5, dur: 1.2, gain: 0.62 },
      ];
      if (bar % 2 === 1) hits.push({ beat: 3.5, dur: 0.45, gain: 0.36 });
    } else if (sec === "bridge") {
      hits = [
        { beat: 0, dur: 3.3, gain: 0.68 },
        { beat: 2.5, dur: 1.1, gain: 0.28 },
      ];
    } else {
      const gain = bar === 19 ? 0.62 : 0.66 - (bar - 16) * 0.06;
      hits = [{ beat: 0, dur: 3.4, gain }];
    }

    for (const hit of hits) {
      for (let ni = 0; ni < chordNotes.length; ni++) {
        renderVoice(target, sends, {
          table: tables.get("Rhodes"),
          note: chordNotes[ni],
          start: at(bar, hit.beat) + ni * 0.011 + jitter(0.005),
          dur: hit.dur * (1 + jitter(0.02)),
          release: 0.55,
          attack: 0.004,
          decay: 1.05,
          sustain: 0.26,
          gain: (hit.gain * (0.92 + rand() * 0.16)) / Math.sqrt(chordNotes.length),
          velocity: 0.9 + rand() * 0.1,
          pan: (ni - (chordNotes.length - 1) / 2) * 0.06,
          unison: 2,
          detune: 3.5,
          stereo: 0.25,
          cutoff: 3100,
          q: 0.82,
          filterEnv: 2.0,
          filterAttack: 0.002,
          filterDecay: 0.3,
          sendRev: 0.32,
          sendDel: 0.05,
        });
      }
    }
  }
}

function schedulePad(target, sends) {
  for (let bar = 0; bar < BARS; bar++) {
    const chord = CHORDS[PROGRESSION[bar]];
    const sec = section(bar);
    const final = bar === BARS - 1;
    const gain =
      sec === "intro" ? 0.3 :
      sec === "main" ? 0.36 :
      sec === "bridge" ? 0.4 :
      final ? 0.34 : 0.3;
    const dur = final ? BAR * 1.6 : BAR * 0.9;
    const notes = chord.notes.slice(1);

    for (let ni = 0; ni < notes.length; ni++) {
      renderVoice(target, sends, {
        table: tables.get("Silk"),
        note: notes[ni],
        start: at(bar, 0) + ni * 0.03 + jitter(0.01),
        dur: dur + jitter(0.05),
        release: final ? 2.8 : 1.9,
        attack: sec === "intro" ? 0.9 : 0.7,
        decay: 1.2,
        sustain: 0.78,
        gain: (gain * (0.9 + rand() * 0.2)) / Math.sqrt(notes.length),
        velocity: 0.85 + rand() * 0.15,
        pan: (ni - (notes.length - 1) / 2) * 0.18,
        unison: 4,
        detune: 11,
        stereo: 0.85,
        cutoff: 1750,
        q: 0.95,
        lfoRate: 0.12 + rand() * 0.05,
        lfoAmount: 0.35,
        sendRev: 0.42,
        randomPhase: true,
      });
    }
  }
}

function scheduleBass(target, sends) {
  let previous = null;

  for (let bar = 0; bar < BARS; bar++) {
    const chord = CHORDS[PROGRESSION[bar]];
    const sec = section(bar);
    const nextChord = CHORDS[PROGRESSION[Math.min(bar + 1, BARS - 1)]];
    const root = chord.bass;
    const events = [];

    if (bar < 2) {
      continue;
    } else if (sec === "intro") {
      events.push({ beat: 0, dur: 3.1, note: root, gain: 0.8 });
      if (bar === 3) events.push({ beat: 2.5, dur: 1.0, note: root + 7, gain: 0.55 });
    } else if (sec === "main") {
      events.push({ beat: 0, dur: 1.5, note: root, gain: 1.0 });
      events.push({
        beat: 2.5,
        dur: 1.0,
        note: bar % 2 === 1 ? root + 7 : root,
        gain: 0.82,
      });
      if (bar % 2 === 1) {
        events.push({ beat: 3.5, dur: 0.35, note: nextChord.bass - 1, gain: 0.48 });
      }
      if (bar % 4 === 0) {
        events.push({ beat: 1.75, dur: 0.3, note: root, gain: 0.32 });
      }
    } else if (sec === "bridge") {
      events.push({ beat: 0, dur: 3.0, note: root, gain: 0.9 });
      events.push({ beat: 2.5, dur: 1.0, note: root + 12, gain: 0.42 });
    } else {
      events.push({ beat: 0, dur: 3.3, note: root, gain: bar === 19 ? 0.9 : 0.78 });
    }

    for (const ev of events) {
      renderVoice(target, sends, {
        table: tables.get("Smooth Sub"),
        note: ev.note,
        start: at(bar, ev.beat) + jitter(0.003),
        dur: ev.dur,
        release: 0.22,
        attack: 0.008,
        decay: 0.3,
        sustain: 0.85,
        gain: ev.gain * 0.76,
        velocity: 0.95 + rand() * 0.05,
        pan: 0,
        unison: 1,
        cutoff: 380,
        q: 0.7,
        filterEnv: 0.9,
        filterAttack: 0.004,
        filterDecay: 0.22,
        glideFrom: previous,
        glideTime: 0.055,
        sendRev: 0.03,
      });
      previous = ev.note;
    }
  }
}

const MELODY = [
  { bar: 7, beat: 3.0, note: 70, dur: 0.5, vel: 0.6 },
  { bar: 7, beat: 3.5, note: 72, dur: 0.5, vel: 0.65 },

  { bar: 8, beat: 0.0, note: 67, dur: 0.75, vel: 0.9 },
  { bar: 8, beat: 0.75, note: 70, dur: 0.75, vel: 0.85 },
  { bar: 8, beat: 1.5, note: 72, dur: 1.0, vel: 0.95 },
  { bar: 8, beat: 2.5, note: 70, dur: 0.5, vel: 0.8 },
  { bar: 8, beat: 3.0, note: 67, dur: 1.0, vel: 0.75 },

  { bar: 9, beat: 0.0, note: 75, dur: 1.0, vel: 0.95 },
  { bar: 9, beat: 1.0, note: 74, dur: 0.5, vel: 0.8 },
  { bar: 9, beat: 1.5, note: 72, dur: 1.0, vel: 0.9 },
  { bar: 9, beat: 2.5, note: 67, dur: 0.5, vel: 0.75 },
  { bar: 9, beat: 3.0, note: 63, dur: 1.0, vel: 0.8 },

  { bar: 10, beat: 0.0, note: 65, dur: 0.75, vel: 0.8 },
  { bar: 10, beat: 0.75, note: 67, dur: 0.75, vel: 0.85 },
  { bar: 10, beat: 1.5, note: 68, dur: 1.0, vel: 0.9 },
  { bar: 10, beat: 2.5, note: 72, dur: 0.5, vel: 0.85 },
  { bar: 10, beat: 3.0, note: 70, dur: 1.0, vel: 0.8 },

  { bar: 11, beat: 0.0, note: 74, dur: 1.0, vel: 0.9 },
  { bar: 11, beat: 1.0, note: 72, dur: 0.75, vel: 0.85 },
  { bar: 11, beat: 1.75, note: 70, dur: 0.75, vel: 0.8 },
  { bar: 11, beat: 2.5, note: 65, dur: 1.5, vel: 0.85 },
  { bar: 11, beat: 3.5, note: 68, dur: 0.5, vel: 0.7 },

  { bar: 12, beat: 0.0, note: 72, dur: 1.0, vel: 0.85 },
  { bar: 12, beat: 1.0, note: 75, dur: 1.0, vel: 0.9 },
  { bar: 12, beat: 2.0, note: 72, dur: 0.5, vel: 0.8 },
  { bar: 12, beat: 2.5, note: 68, dur: 1.0, vel: 0.85 },
  { bar: 12, beat: 3.5, note: 67, dur: 0.5, vel: 0.75 },

  { bar: 13, beat: 0.0, note: 65, dur: 1.0, vel: 0.8 },
  { bar: 13, beat: 1.0, note: 68, dur: 1.5, vel: 0.85 },
  { bar: 13, beat: 2.5, note: 72, dur: 0.5, vel: 0.8 },
  { bar: 13, beat: 3.0, note: 75, dur: 1.0, vel: 0.9 },

  { bar: 14, beat: 0.0, note: 73, dur: 1.0, vel: 0.85 },
  { bar: 14, beat: 1.0, note: 72, dur: 0.5, vel: 0.8 },
  { bar: 14, beat: 1.5, note: 70, dur: 1.0, vel: 0.85 },
  { bar: 14, beat: 2.5, note: 65, dur: 0.5, vel: 0.75 },
  { bar: 14, beat: 3.0, note: 68, dur: 1.0, vel: 0.8 },

  { bar: 15, beat: 0.0, note: 67, dur: 0.75, vel: 0.85 },
  { bar: 15, beat: 0.75, note: 70, dur: 0.75, vel: 0.85 },
  { bar: 15, beat: 1.5, note: 73, dur: 1.0, vel: 0.9 },
  { bar: 15, beat: 2.5, note: 72, dur: 1.5, vel: 0.9 },

  { bar: 18, beat: 0.0, note: 75, dur: 1.5, vel: 0.65 },
  { bar: 19, beat: 0.0, note: 75, dur: 3.4, vel: 0.55 },
  { bar: 19, beat: 0.0, note: 70, dur: 3.4, vel: 0.45 },
];

function scheduleLead(target, sends) {
  for (const ev of MELODY) {
    const start = at(ev.bar, ev.beat) + jitter(0.006);
    const dur = ev.dur * BEAT * (1 + jitter(0.015));

    renderVoice(target, sends, {
      table: tables.get("Sine"),
      note: ev.note,
      start,
      dur,
      release: 0.75,
      attack: 0.012,
      decay: 0.4,
      sustain: 0.55,
      gain: 0.42 * ev.vel,
      velocity: 1,
      pan: jitter(0.08),
      unison: 2,
      detune: 5,
      stereo: 0.22,
      cutoff: 4200,
      q: 1.0,
      lfoRate: 0.18,
      lfoAmount: 0.12,
      vibratoRate: 5.2,
      vibratoCents: 9,
      sendRev: 0.5,
      sendDel: 0.28,
    });

    renderVoice(target, sends, {
      table: tables.get("Sine"),
      note: ev.note + 12,
      start,
      dur: dur * 0.7,
      release: 0.5,
      attack: 0.008,
      decay: 0.25,
      sustain: 0.2,
      gain: 0.11 * ev.vel,
      velocity: 1,
      pan: jitter(0.12),
      unison: 1,
      cutoff: 6000,
      q: 0.8,
      sendRev: 0.55,
      sendDel: 0.2,
    });

    renderVoice(target, sends, {
      table: tables.get("Saw Retro"),
      note: ev.note + 12,
      start,
      dur: dur * 0.55,
      release: 0.35,
      attack: 0.004,
      decay: 0.18,
      sustain: 0.08,
      gain: 0.055 * ev.vel,
      velocity: 1,
      pan: jitter(0.18),
      unison: 1,
      cutoff: 3000,
      q: 0.7,
      filterEnv: 1.2,
      filterDecay: 0.15,
      sendRev: 0.5,
      sendDel: 0.22,
    });
  }
}

function scheduleArp(target, sends) {
  for (let bar = 4; bar < 16; bar++) {
    const chord = CHORDS[PROGRESSION[bar]];
    const pool = chord.notes
      .map((n) => {
        let v = n;
        while (v < 60) v += 12;
        return v;
      })
      .sort((a, b) => a - b);

    for (let s = 0; s < 8; s++) {
      const swing = s % 2 === 1 ? 0.065 : 0;
      const idx = (s < 4 ? [0, 2, 1, 3] : [2, 4, 3, 1])[s % 4];
      const note = pool[idx % pool.length] + (s === 5 ? 12 : 0);

      renderVoice(target, sends, {
        table: tables.get("Saw Retro"),
        note,
        start: at(bar, s * 0.5 + swing) + jitter(0.004),
        dur: 0.16,
        release: 0.24,
        attack: 0.002,
        decay: 0.16,
        sustain: 0,
        gain: 0.21,
        velocity: 0.8 + rand() * 0.2,
        pan: s % 2 === 0 ? -0.55 : 0.55,
        unison: 1,
        cutoff: 2300,
        q: 1.3,
        filterEnv: 2.6,
        filterAttack: 0.002,
        filterDecay: 0.12,
        sendRev: 0.28,
        sendDel: 0.2,
      });
    }
  }
}

function fadeOut(t, len) {
  return clamp((len - t) / 0.02, 0, 1);
}

function addKick(target, sends, start, gain) {
  const len = 0.5;
  const i0 = Math.max(0, Math.floor(start * SR));
  const i1 = Math.min(N, i0 + Math.floor(len * SR));
  let phase = 0;
  for (let i = i0; i < i1; i++) {
    const t = (i - i0) / SR;
    const f = 46 + 100 * Math.exp(-t / 0.032);
    phase += (2 * Math.PI * f) / SR;
    const env = Math.exp(-t / 0.15) * fadeOut(t, len);
    const click = Math.exp(-t / 0.004) * (rand() * 2 - 1) * 0.35;
    const x = (Math.sin(phase) * env + click) * gain;
    target.l[i] += x;
    target.r[i] += x;
    sends.rev.l[i] += x * 0.04;
    sends.rev.r[i] += x * 0.04;
  }
}

function addSnare(target, sends, start, gain) {
  const len = 0.35;
  const i0 = Math.max(0, Math.floor(start * SR));
  const i1 = Math.min(N, i0 + Math.floor(len * SR));
  let lpL = 0;
  let lpR = 0;
  for (let i = i0; i < i1; i++) {
    const t = (i - i0) / SR;
    const nL = rand() * 2 - 1;
    const nR = rand() * 2 - 1;
    lpL += (nL - lpL) * 0.25;
    lpR += (nR - lpR) * 0.25;
    const hpL = nL - lpL;
    const hpR = nR - lpR;
    const tone =
      Math.sin(2 * Math.PI * 185 * t) * Math.exp(-t / 0.05) +
      Math.sin(2 * Math.PI * 330 * t) * 0.5 * Math.exp(-t / 0.035);
    const env = Math.exp(-t / 0.1) * fadeOut(t, len) * (1 - Math.exp(-t / 0.001));
    const xL = (tone * 0.55 + hpL * 0.9) * env * gain;
    const xR = (tone * 0.55 + hpR * 0.9) * env * gain;
    target.l[i] += xL;
    target.r[i] += xR;
    sends.rev.l[i] += xL * 0.12;
    sends.rev.r[i] += xR * 0.12;
  }
}

function addClap(target, sends, start, gain) {
  const len = 0.28;
  const bursts = [0, 0.011, 0.022, 0.033];
  const i0 = Math.max(0, Math.floor(start * SR));
  const i1 = Math.min(N, i0 + Math.floor(len * SR));
  let lpL = 0;
  let lpR = 0;
  for (let i = i0; i < i1; i++) {
    const t = (i - i0) / SR;
    let env = 0;
    for (const b of bursts) {
      const dt = t - b;
      if (dt >= 0) env = Math.max(env, Math.exp(-dt / 0.018));
    }
    env = (env + Math.exp(-t / 0.13) * 0.22) * fadeOut(t, len);
    const nL = rand() * 2 - 1;
    const nR = rand() * 2 - 1;
    lpL += (nL - lpL) * 0.2;
    lpR += (nR - lpR) * 0.2;
    const xL = (nL - lpL) * env * gain;
    const xR = (nR - lpR) * env * gain;
    target.l[i] += xL;
    target.r[i] += xR;
    sends.rev.l[i] += xL * 0.2;
    sends.rev.r[i] += xR * 0.2;
  }
}

function addHat(target, sends, start, gain, open) {
  const len = open ? 0.4 : 0.08;
  const i0 = Math.max(0, Math.floor(start * SR));
  const i1 = Math.min(N, i0 + Math.floor(len * SR));
  let lpL = 0;
  let lpR = 0;
  for (let i = i0; i < i1; i++) {
    const t = (i - i0) / SR;
    const nL = rand() * 2 - 1;
    const nR = rand() * 2 - 1;
    lpL += (nL - lpL) * 0.5;
    lpR += (nR - lpR) * 0.5;
    const env = Math.exp(-t / (open ? 0.13 : 0.028)) * fadeOut(t, len);
    const xL = (nL - lpL) * env * gain;
    const xR = (nR - lpR) * env * gain;
    target.l[i] += xL;
    target.r[i] += xR;
    sends.rev.l[i] += xL * 0.08;
    sends.rev.r[i] += xR * 0.08;
  }
}

function scheduleDrums(target, sends) {
  for (let bar = 0; bar < BARS; bar++) {
    const sec = section(bar);

    if (bar < 3) continue;

    if (bar === 3) {
      addKick(target, sends, at(bar, 0), 0.55);
      addKick(target, sends, at(bar, 2.5), 0.45);
      for (let s = 0; s < 8; s++) addHat(target, sends, at(bar, s * 0.5), 0.1 + s * 0.008, false);
      for (let s = 0; s < 8; s++) addSnare(target, sends, at(bar, 2 + s * 0.25), 0.05 + s * 0.022, false);
      continue;
    }

    const fade =
      sec === "outro"
        ? bar === 16 ? 0.85 : bar === 17 ? 0.6 : bar === 18 ? 0.4 : 0
        : 1;
    if (fade <= 0) continue;

    addKick(target, sends, at(bar, 0), 0.82 * fade);
    addKick(target, sends, at(bar, 2.5), 0.72 * fade);
    if (bar % 2 === 1) addKick(target, sends, at(bar, 1.75), 0.3 * fade);
    if (bar % 4 === 3 && sec !== "outro") addKick(target, sends, at(bar, 3.75), 0.34 * fade);

    addSnare(target, sends, at(bar, 1), 0.82 * fade);
    addSnare(target, sends, at(bar, 3), 0.88 * fade);
    addClap(target, sends, at(bar, 1), 0.42 * fade);
    addClap(target, sends, at(bar, 3), 0.48 * fade);
    if (bar % 2 === 0 && sec !== "outro") addSnare(target, sends, at(bar, 3.75), 0.2 * fade);

    for (let s = 0; s < 16; s++) {
      const swing = s % 2 === 1 ? 0.055 : 0;
      const accent = s % 4 === 0 ? 0.24 : s % 2 === 0 ? 0.16 : 0.1;
      addHat(target, sends, at(bar, s * 0.25 + swing), accent * fade, false);
    }
    if (bar % 2 === 1) addHat(target, sends, at(bar, 3.5), 0.2 * fade, true);

    if (bar === 11 || bar === 15) {
      for (let s = 0; s < 4; s++) {
        addSnare(target, sends, at(bar, 3 + s * 0.25), (0.25 + s * 0.16) * fade, false);
      }
    }
  }

  addKick(target, sends, at(19, 0), 0.7);
  addHat(target, sends, at(19, 0), 0.16, true);
}

class Comb {
  constructor(size, feedback, damp) {
    this.buf = new Float32Array(Math.max(1, Math.floor(size)));
    this.idx = 0;
    this.store = 0;
    this.feedback = feedback;
    this.damp = damp;
  }
  process(x) {
    const out = this.buf[this.idx];
    this.store = out * (1 - this.damp) + this.store * this.damp;
    this.buf[this.idx] = x + this.store * this.feedback;
    this.idx = (this.idx + 1) % this.buf.length;
    return out;
  }
}

class Allpass {
  constructor(size, feedback) {
    this.buf = new Float32Array(Math.max(1, Math.floor(size)));
    this.idx = 0;
    this.feedback = feedback;
  }
  process(x) {
    const bufout = this.buf[this.idx];
    const out = -x + bufout;
    this.buf[this.idx] = x + bufout * this.feedback;
    this.idx = (this.idx + 1) % this.buf.length;
    return out;
  }
}

function buildReverb(source) {
  const scale = SR / 44100;
  const spread = 23;
  const combTunings = [1116, 1188, 1277, 1356, 1422, 1491];
  const apTunings = [556, 441];
  const combsL = combTunings.map((t) => new Comb(t * scale, 0.86, 0.22));
  const combsR = combTunings.map((t) => new Comb((t + spread) * scale, 0.86, 0.22));
  const apL = apTunings.map((t) => new Allpass(t * scale, 0.5));
  const apR = apTunings.map((t) => new Allpass((t + spread) * scale, 0.5));
  const outL = new Float32Array(N);
  const outR = new Float32Array(N);

  for (let i = 0; i < N; i++) {
    const x = (source.l[i] + source.r[i]) * 0.5 * 0.03;
    let l = 0;
    let r = 0;
    for (const c of combsL) l += c.process(x);
    for (const c of combsR) r += c.process(x);
    for (const a of apL) l = a.process(l);
    for (const a of apR) r = a.process(r);
    outL[i] = l * 2.2;
    outR[i] = r * 2.2;
  }
  return { l: outL, r: outR };
}

function buildDelay(source) {
  const delaySamples = Math.max(1, Math.floor(BEAT * 0.75 * SR));
  const bufL = new Float32Array(delaySamples);
  const bufR = new Float32Array(delaySamples);
  const outL = new Float32Array(N);
  const outR = new Float32Array(N);
  let idx = 0;
  let lpL = 0;
  let lpR = 0;

  for (let i = 0; i < N; i++) {
    const dl = bufL[idx];
    const dr = bufR[idx];
    outL[i] = dl * 0.7;
    outR[i] = dr * 0.7;
    lpL += (dr - lpL) * 0.35;
    lpR += (dl - lpR) * 0.35;
    bufL[idx] = source.l[i] + lpL * 0.38;
    bufR[idx] = source.r[i] + lpR * 0.38;
    idx = (idx + 1) % delaySamples;
  }
  return { l: outL, r: outR };
}

function rms(buf) {
  let sum = 0;
  for (let i = 0; i < buf.length; i++) sum += buf[i] * buf[i];
  return Math.sqrt(sum / buf.length);
}

function wavHeader(dataSize, channels, sampleRate, bits) {
  const b = Buffer.alloc(44);
  b.write("RIFF", 0);
  b.writeUInt32LE(36 + dataSize, 4);
  b.write("WAVE", 8);
  b.write("fmt ", 12);
  b.writeUInt32LE(16, 16);
  b.writeUInt16LE(1, 20);
  b.writeUInt16LE(channels, 22);
  b.writeUInt32LE(sampleRate, 24);
  b.writeUInt32LE(sampleRate * channels * (bits / 8), 28);
  b.writeUInt16LE(channels * (bits / 8), 32);
  b.writeUInt16LE(bits, 34);
  b.write("data", 36);
  b.writeUInt32LE(dataSize, 40);
  return b;
}

function main() {
  const started = Date.now();
  tables = loadTables();

  const drums = bus();
  const bass = bus();
  const keys = bus();
  const pad = bus();
  const lead = bus();
  const arp = bus();
  const sends = sendsObject();

  scheduleDrums(drums, sends);
  scheduleBass(bass, sends);
  scheduleKeys(keys, sends);
  schedulePad(pad, sends);
  scheduleLead(lead, sends);
  scheduleArp(arp, sends);

  console.log("dry RMS", {
    drums: rms(drums.l).toFixed(4),
    bass: rms(bass.l).toFixed(4),
    keys: rms(keys.l).toFixed(4),
    pad: rms(pad.l).toFixed(4),
    lead: rms(lead.l).toFixed(4),
    arp: rms(arp.l).toFixed(4),
  });

  const reverb = buildReverb(sends.rev);
  const delay = buildDelay(sends.del);
  console.log("send RMS", {
    reverb: rms(reverb.l).toFixed(4),
    delay: rms(delay.l).toFixed(4),
  });

  const mixL = new Float32Array(N);
  const mixR = new Float32Array(N);

  const fadeStart = BARS * BAR + 1.0;
  let hpPrevL = 0;
  let hpPrevR = 0;
  let hpOutL = 0;
  let hpOutR = 0;
  const hpR = Math.exp((-2 * Math.PI * 25) / SR);
  let shelfLpL = 0;
  let shelfLpR = 0;
  const shelfA = 1 - Math.exp((-2 * Math.PI * 2800) / SR);

  for (let i = 0; i < N; i++) {
    let l =
      drums.l[i] * 1.0 +
      bass.l[i] * 0.78 +
      keys.l[i] * 0.82 +
      pad.l[i] * 0.55 +
      lead.l[i] * 0.9 +
      arp.l[i] * 0.7 +
      reverb.l[i] * 0.75 +
      delay.l[i] * 0.6;
    let r =
      drums.r[i] * 1.0 +
      bass.r[i] * 0.78 +
      keys.r[i] * 0.82 +
      pad.r[i] * 0.55 +
      lead.r[i] * 0.9 +
      arp.r[i] * 0.7 +
      reverb.r[i] * 0.75 +
      delay.r[i] * 0.6;

    const hpL = l - hpPrevL + hpR * hpOutL;
    hpPrevL = l;
    hpOutL = hpL;
    const hpRr = r - hpPrevR + hpR * hpOutR;
    hpPrevR = r;
    hpOutR = hpRr;

    const mid = (hpL + hpRr) * 0.5;
    const side = (hpL - hpRr) * 0.5;
    let wl = mid + side * 1.22;
    let wr = mid - side * 1.22;

    shelfLpL += (wl - shelfLpL) * shelfA;
    shelfLpR += (wr - shelfLpR) * shelfA;
    wl += (wl - shelfLpL) * 0.3;
    wr += (wr - shelfLpR) * 0.3;

    l = Math.tanh(wl * 1.12);
    r = Math.tanh(wr * 1.12);

    const t = i / SR;
    if (t > fadeStart) {
      const x = clamp((t - fadeStart) / (TOTAL - fadeStart), 0, 1);
      const f = 0.5 * (1 + Math.cos(Math.PI * x));
      l *= f;
      r *= f;
    }

    if (!Number.isFinite(l) || !Number.isFinite(r)) {
      throw new Error("non-finite sample at " + i);
    }

    mixL[i] = l;
    mixR[i] = r;
  }

  let peak = 0;
  for (let i = 0; i < N; i++) {
    const a = Math.abs(mixL[i]);
    const b = Math.abs(mixR[i]);
    if (a > peak) peak = a;
    if (b > peak) peak = b;
  }
  const scale = peak > 0 ? 0.93 / peak : 1;
  const data = Buffer.alloc(N * 4);
  for (let i = 0; i < N; i++) {
    const l = clamp(mixL[i] * scale, -1, 1);
    const r = clamp(mixR[i] * scale, -1, 1);
    data.writeInt16LE(Math.round(l * 32767), i * 4);
    data.writeInt16LE(Math.round(r * 32767), i * 4 + 2);
  }

  if (!fs.existsSync(OUT_DIR)) fs.mkdirSync(OUT_DIR, { recursive: true });
  const outPath = path.join(OUT_DIR, OUT_NAME);
  fs.writeFileSync(outPath, wavHeader(data.length, 2, SR, 16));
  fs.appendFileSync(outPath, data);

  console.log("wrote", outPath);
  console.log("duration", (N / SR).toFixed(2), "s", "peak-before-norm", peak.toFixed(4));
  console.log("render time", ((Date.now() - started) / 1000).toFixed(2), "s");
}

main();
