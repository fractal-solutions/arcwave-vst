#!/usr/bin/env node
// Clean-room reference renderer for the reverse-engineered GMS architecture.
// Loads the user's locally extracted wavetables and renders a short phrase.
// No Image-Line code or assets are embedded; tables are read from assets/.

const fs = require("fs");
const path = require("path");

const ROOT = path.resolve(__dirname, "..");
const WT_DIR = path.join(ROOT, "assets", "wavetables");
const OUT_DIR = path.join(ROOT, "out");
const SAMPLE_RATE = 48000;

function loadIndex() {
  const raw = fs.readFileSync(path.join(WT_DIR, "index.json"), "utf8").replace(/^\uFEFF/, "");
  const idx = JSON.parse(raw);
  const byName = new Map();
  for (const w of idx.wavetables) {
    const buf = fs.readFileSync(path.join(WT_DIR, w.raw_f32));
    const src = new Float32Array(w.samples);
    for (let i = 0; i < w.samples; i++) src[i] = buf.readFloatLE(i * 4);
    byName.set(w.name, resampleCycle(src, 2048));
  }
  return byName;
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

function readTable(table, phase) {
  const x = (phase - Math.floor(phase)) * (table.length - 1);
  const i = Math.floor(x);
  const t = x - i;
  return table[i] * (1 - t) + table[i + 1] * t;
}

class SVF {
  constructor() { this.ic1 = 0; this.ic2 = 0; }
  process(x, cutoff, q) {
    const g = Math.tan((Math.PI * Math.min(cutoff, SAMPLE_RATE * 0.45)) / SAMPLE_RATE);
    const k = 1 / q;
    const a1 = 1 / (1 + g * (g + k));
    const a2 = g * a1;
    const a3 = g * a2;
    const v3 = x - this.ic2;
    const v1 = a1 * this.ic1 + a2 * v3;
    const v2 = this.ic2 + a2 * this.ic1 + a3 * v3;
    this.ic1 = 2 * v1 - this.ic1;
    this.ic2 = 2 * v2 - this.ic2;
    return { lp: v2, hp: x - k * v1 - v2 };
  }
}

function midiToHz(n) { return 440 * Math.pow(2, (n - 69) / 12); }

class Voice {
  constructor(ctx, note, start, dur) {
    this.ctx = ctx;
    this.note = note;
    this.start = start;
    this.dur = dur;
    this.releaseStart = start + dur;
    this.end = start + dur + ctx.params.release;
    this.phases = [0, 1, 2].map(() => new Float32Array(ctx.params.unison));
    this.filter = new SVF();
    this.env = 0;
    this.oscs = [
      { table: ctx.tables.get(ctx.params.wave1), mix: 1.0, detune: 0 },
      { table: ctx.tables.get(ctx.params.wave2), mix: ctx.params.mix2, detune: ctx.params.detune2 },
      { table: ctx.tables.get(ctx.params.wave3), mix: ctx.params.mix3, detune: ctx.params.detune3 },
    ];
  }
  render(outL, outR, i, t) {
    const p = this.ctx.params;
    if (t < this.start || t >= this.end) return false;

    let target;
    if (t < this.start + p.attack) target = (t - this.start) / p.attack;
    else if (t < this.start + p.attack + p.decay) target = 1 - (1 - p.sustain) * ((t - this.start - p.attack) / p.decay);
    else if (t < this.releaseStart) target = p.sustain;
    else target = p.sustain * Math.max(0, 1 - (t - this.releaseStart) / p.release);
    this.env += (target - this.env) * 0.01;

    const lfo = Math.sin(2 * Math.PI * p.lfoRate * t) * p.lfoAmount;
    let cutoff = p.cutoff * Math.pow(2, lfo * 2);

    let l = 0;
    let r = 0;
    for (let o = 0; o < 3; o++) {
      const osc = this.oscs[o];
      if (osc.mix <= 0) continue;
      for (let v = 0; v < p.unison; v++) {
        const spread = p.unison > 1 ? (v / (p.unison - 1)) * 2 - 1 : 0;
        const cents = spread * p.unisonDetune * 100 + osc.detune;
        const freq = midiToHz(this.note + p.octave * 12) * Math.pow(2, cents / 1200);
        let ph = this.phases[o][v];
        const s = readTable(osc.table, ph) * osc.mix / p.unison;
        const pan = spread * p.unisonStereo;
        l += s * Math.sqrt((1 - pan) / 2);
        r += s * Math.sqrt((1 + pan) / 2);
        ph += freq / SAMPLE_RATE;
        this.phases[o][v] = ph - Math.floor(ph);
      }
    }

    const noise = (Math.random() * 2 - 1) * p.noise;
    l += noise;
    r += noise;

    const filteredL = this.filter.process(l, cutoff, p.resonance).lp;
    const filteredR = this.filter.process(r, cutoff, p.resonance).lp;

    outL[i] += filteredL * this.env;
    outR[i] += filteredR * this.env;
    return true;
  }
}

function main() {
  const tables = loadIndex();
  const params = {
    wave1: "Saw", wave2: "Square", wave3: "Sine",
    mix2: 0.45, mix3: 0.2, detune2: 7, detune3: -7,
    octave: 0, unison: 5, unisonDetune: 0.12, unisonStereo: 0.6,
    noise: 0.02, cutoff: 2400, resonance: 0.35,
    attack: 0.008, decay: 0.25, sustain: 0.7, release: 0.35,
    lfoRate: 5.0, lfoAmount: 0.15,
  };

  const ctx = { tables, params };
  const phrase = [
    { note: 36, start: 0.0, dur: 0.55 },
    { note: 43, start: 0.6, dur: 0.55 },
    { note: 48, start: 1.2, dur: 0.55 },
    { note: 55, start: 1.8, dur: 0.9 },
    { note: 36, start: 2.8, dur: 1.2 },
  ];
  const seconds = 4.4;
  const n = Math.floor(seconds * SAMPLE_RATE);
  const outL = new Float32Array(n);
  const outR = new Float32Array(n);

  const voices = phrase.map((p) => new Voice(ctx, p.note, p.start, p.dur));
  for (let i = 0; i < n; i++) {
    const t = i / SAMPLE_RATE;
    for (const v of voices) v.render(outL, outR, i, t);
  }

  let peak = 0;
  for (let i = 0; i < n; i++) peak = Math.max(peak, Math.abs(outL[i]), Math.abs(outR[i]));
  const g = peak > 0 ? 0.89 / peak : 1;

  if (!fs.existsSync(OUT_DIR)) fs.mkdirSync(OUT_DIR, { recursive: true });
  const outPath = path.join(OUT_DIR, "demo.wav");
  const data = Buffer.alloc(n * 4);
  for (let i = 0; i < n; i++) {
    data.writeInt16LE(Math.round(Math.max(-1, Math.min(1, outL[i] * g)) * 32767), i * 4);
    data.writeInt16LE(Math.round(Math.max(-1, Math.min(1, outR[i] * g)) * 32767), i * 4 + 2);
  }
  fs.writeFileSync(outPath, wavHeader(n * 4, 2, SAMPLE_RATE, 16).toString("binary"), "binary");
  fs.appendFileSync(outPath, data);
  console.log("tables loaded:", [...tables.keys()].join(", "));
  console.log("wrote", outPath, (fs.statSync(outPath).size / 1024).toFixed(1) + " KiB", "peak", peak.toFixed(3));
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

main();
