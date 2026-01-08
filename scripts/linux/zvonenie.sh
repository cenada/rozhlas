#!/usr/bin/env bash
set -euo pipefail

# ====== Configuration ======
START_TIMES=( "07:40" "08:30" "09:25" "10:20" "11:25" "12:15" "13:05" "14:00" )
END_TIMES=(   "08:25" "09:15" "10:10" "11:05" "12:10" "13:00" "13:45" "14:45" )

SERIAL_DEV="/dev/ttyUSB0"

# Audio: explicit device + software gain (your working settings)
ADEV="plughw:1,0"
MPG123_GAIN="7000"

# Warmup to avoid the HDMI "shoot"
WARMUP_SEC="8"

# How long time after start of warm up to start turning radio
TURNON_AFTER_SEC="4"

# And how long time to wait after turning the radio on before playing music
POST_ON_DELAY="5"

# If you want to disable ringing on weekends:
WEEKDAYS_ONLY="1"   # 1=yes (Mon-Fri), 0=no

# ====== Internals ======
BASE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROZHLAS_BIN="${BASE_DIR}/rozhlas"

log() { echo "[$(date '+%F %T')] $*"; }

in_list() {
  local needle="$1"; shift
  local x
  for x in "$@"; do
    [[ "$x" == "$needle" ]] && return 0
  done
  return 1
}

warm_hdmi() {
  # Play silence for a few seconds to wake HDMI/extractor path (avoid pop)
  # timeout "${WARMUP_SEC}" \
  aplay -q -d "${WARMUP_SEC}" -D "${ADEV}" -f S16_LE -r 48000 -c 2 /dev/zero >/dev/null 2>&1 || true &
}

radio_on()  { "${ROZHLAS_BIN}" "${SERIAL_DEV}" on;  }
radio_off() { "${ROZHLAS_BIN}" "${SERIAL_DEV}" off; }

cleanup() {
  # Best-effort safety shutdown
  radio_off >/dev/null 2>&1 || true
}
trap cleanup EXIT INT TERM

play_folder_mp3() {
  local folder="$1"   # "zaciatok" or "koniec"
  local dir="${BASE_DIR}/${folder}"

  if ! compgen -G "${dir}/*.mp3" > /dev/null; then
    log "WARN: No MP3 files found in ${dir}"
    return 0
  fi

  warm_hdmi
  sleep "${TURNON_AFTER_SEC}"
  radio_on
  sleep "${POST_ON_DELAY}"

  # Play all mp3 in that folder (sorted by name). If you want random, tell me.
  ( cd -- "${dir}" && mpg123 -q -o alsa -a "${ADEV}" -f "${MPG123_GAIN}" ./*.mp3 )

  sleep 1
  warm_hdmi
  sleep "${TURNON_AFTER_SEC}" 
  radio_off

  # Prevent multiple plays within the same minute (like BAT)
  sleep 61
}

log "School bell running. Start times: ${START_TIMES[*]}"
log "                 End times:   ${END_TIMES[*]}"
log "Press Ctrl-C to stop."

while true; do
  if [[ "${WEEKDAYS_ONLY}" == "1" ]]; then
    dow="$(date +%u)"   # 1..7 (Mon..Sun)
    if [[ "${dow}" -ge 6 ]]; then
      sleep 30
      continue
    fi
  fi

  now="$(date +%H:%M)"

  if in_list "${now}" "${START_TIMES[@]}"; then
    log "Ringing START (${now})"
    play_folder_mp3 "zaciatok"
    continue
  fi

  if in_list "${now}" "${END_TIMES[@]}"; then
    log "Ringing END (${now})"
    play_folder_mp3 "koniec"
    continue
  fi

  sleep 1
done

