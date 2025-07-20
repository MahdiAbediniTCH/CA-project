#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 4 ]]; then
  echo "Usage: $0 <trace_file> <warmup_count> <sim_count> <policy>"
  exit 1
fi

CONFIG_FILE="cassandra_phase0_core0.trace.xz"
TRACE_FILE=$1
WARMUP_COUNT=$2
SIM_COUNT=$3
POLICY=$4

# Paired lists: each index i yields one (SETS[i], WAYS[i]) pair
SETS=(1024 2048 4096 8192 16384 32768)
WAYS=(32   16    8    4     2      1)

# Helper to update the JSON in-place
#   $1 = jq-filter, rest = any --arg / --argjson flags for jq
update_json() {
  local filter="$1"; shift
  jq "$@" "$filter" "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" \
    && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
}

# 1) Set the replacement policy once
update_json '.LLC.replacement = $pol' --arg pol "$POLICY"


# 2) Sweep through each (sets, ways) pair
for i in "${!SETS[@]}"; do
  set=${SETS[i]}
  way=${WAYS[i]}

  # patch in the new set & way
  update_json '.LLC.sets = $s | .LLC.ways = $w' \
    --argjson s "$set" --argjson w "$way"
  echo "+ Running ${POLICY}_${set}_${way}"
  ./config.sh "$CONFIG_FILE"
  make > /dev/null

  # launch the sim in the background
  bin/champsim \
    --warmup-instructions "$WARMUP_COUNT" \
    --simulation-instructions "$SIM_COUNT" \
    "$TRACE_FILE" \
    > "${POLICY}_${set}_${way}.txt" && echo "- ${POLICY}_${set}_${way}.txt"&
done

wait

