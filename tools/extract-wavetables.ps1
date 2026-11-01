param(
    [string]$GmsDir = "C:\Program Files\Image-Line\FL Studio 2025\Plugins\Fruity\Generators\GMS",
    [string]$OutDir = "C:\dev\vst\assets\wavetables"
)

$ErrorActionPreference = "Stop"

$waveDir = Join-Path $GmsDir "Synth Waves"
if (-not (Test-Path -LiteralPath $waveDir)) {
    throw "Wavetable directory not found: $waveDir"
}
if (-not (Test-Path -LiteralPath $OutDir)) {
    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
}

function Read-Wavetable([string]$path) {
    $bytes = [System.IO.File]::ReadAllBytes($path)
    $id = [System.Text.Encoding]::ASCII.GetString($bytes, 0, 4)
    if ($id -ne "RIFF") { throw "Not a RIFF file: $path" }

    $audioFormat = 0
    $channels = 0
    $sampleRate = 0
    $bits = 0
    $dataOffset = -1
    $dataSize = 0

    $p = 12
    while ($p + 8 -le $bytes.Length) {
        $chunkId = [System.Text.Encoding]::ASCII.GetString($bytes, $p, 4)
        $chunkSize = [BitConverter]::ToUInt32($bytes, $p + 4)
        if ($chunkId -eq "fmt ") {
            $audioFormat = [BitConverter]::ToUInt16($bytes, $p + 8)
            $channels = [BitConverter]::ToUInt16($bytes, $p + 10)
            $sampleRate = [BitConverter]::ToUInt32($bytes, $p + 12)
            $bits = [BitConverter]::ToUInt16($bytes, $p + 22)
        }
        elseif ($chunkId -eq "data") {
            $dataOffset = $p + 8
            $dataSize = $chunkSize
        }
        $p += 8 + [int]$chunkSize + ([int]$chunkSize % 2)
    }

    if ($dataOffset -lt 0) { throw "No data chunk: $path" }

    $bytesPerSample = [int]($bits / 8)
    $frameSize = $bytesPerSample * $channels
    $frameCount = [int]($dataSize / $frameSize)
    $samples = New-Object 'System.Collections.Generic.List[float]'

    for ($f = 0; $f -lt $frameCount; $f++) {
        $sum = 0.0
        $base = $dataOffset + ($f * $frameSize)
        for ($c = 0; $c -lt $channels; $c++) {
            $o = $base + ($c * $bytesPerSample)
            if ($audioFormat -eq 3 -and $bits -eq 32) {
                $sum += [BitConverter]::ToSingle($bytes, $o)
            }
            elseif ($audioFormat -eq 1 -and $bits -eq 16) {
                $sum += [BitConverter]::ToInt16($bytes, $o) / 32768.0
            }
            elseif ($audioFormat -eq 1 -and $bits -eq 32) {
                $sum += [BitConverter]::ToInt32($bytes, $o) / 2147483648.0
            }
            elseif ($audioFormat -eq 1 -and $bits -eq 24) {
                $v = $bytes[$o] -bor ($bytes[$o + 1] -shl 8) -bor ($bytes[$o + 2] -shl 16)
                if ($v -band 0x800000) { $v = $v -bor 0xFF000000 }
                $sum += $v / 8388608.0
            }
            else {
                throw "Unsupported WAV format ($audioFormat/$bits bit) in $path"
            }
        }
        $samples.Add([float]($sum / $channels))
    }

    $peak = 0.0
    foreach ($s in $samples) { $a = [Math]::Abs($s); if ($a -gt $peak) { $peak = $a } }

    return [pscustomobject]@{
        Name       = [System.IO.Path]::GetFileNameWithoutExtension($path)
        File       = [System.IO.Path]::GetFileName($path)
        AudioFormat = $audioFormat
        Channels   = $channels
        SampleRate = $sampleRate
        Bits       = $bits
        Samples    = $samples.Count
        Peak       = [Math]::Round($peak, 4)
        Values     = $samples
    }
}

$index = New-Object 'System.Collections.Generic.List[object]'

Get-ChildItem -LiteralPath $waveDir -Filter *.wav | Sort-Object Name | ForEach-Object {
    $wt = Read-Wavetable $_.FullName
    $rawName = "$($wt.Name).f32"
    $rawPath = Join-Path $OutDir $rawName
    $fs = [System.IO.File]::Create($rawPath)
    $bw = New-Object System.IO.BinaryWriter($fs)
    try {
        foreach ($v in $wt.Values) { $bw.Write([single]$v) }
    }
    finally {
        $bw.Dispose()
    }

    $entry = [ordered]@{
        name        = $wt.Name
        source_file = $wt.File
        raw_f32     = $rawName
        sample_rate = $wt.SampleRate
        channels    = $wt.Channels
        bits        = $wt.Bits
        format      = $wt.AudioFormat
        samples     = $wt.Samples
        peak        = $wt.Peak
    }
    $index.Add([pscustomobject]$entry)
    "{0,-16} {1,5} samples  {2}/{3}bit  peak {4}" -f $wt.Name, $wt.Samples, $wt.AudioFormat, $wt.Bits, $wt.Peak
}

$manifest = [ordered]@{
    generated_from = $waveDir
    note           = "Derived from wavetables in the user's local GMS installation. Do not redistribute these samples."
    format         = "raw little-endian float32 mono, one value per original sample frame"
    count          = $index.Count
    wavetables     = $index
}
$manifestPath = Join-Path $OutDir "index.json"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($manifestPath, ($manifest | ConvertTo-Json -Depth 5), $utf8NoBom)
Write-Output "Wrote $($index.Count) wavetables to $OutDir"
Write-Output "Manifest: $manifestPath"
