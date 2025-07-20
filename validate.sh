#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <Policy>"
  exit 1
fi

CONFIG_FILE="champsim_config.json"
WARMUP_COUNT=800
SIM_COUNT=2400
POLICY=$1

# Helper to update the JSON in-place
#   $1 = jq-filter, rest = any --arg / --argjson flags for jq
update_json() {
  local filter="$1"; shift
  jq "$@" "$filter" "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" \
    && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
}

# 1) Set the replacement policy once
update_json '.LLC.replacement = $pol' --arg pol "$POLICY"

set=2048
way=16

# patch in the new set & way
update_json '.LLC.sets = $s | .LLC.ways = $w' \
  --argjson s "$set" --argjson w "$way"
echo "+ Running ${POLICY}_${set}_${way}"
./config.sh "$CONFIG_FILE"
make

# launch the sim in the background
bin/champsim \
  --warmup-instructions "$WARMUP_COUNT" \
  --simulation-instructions "$SIM_COUNT" \
  cassandra_phase0_core0.trace.xz \

wait

