#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "Usage: $0 <trace_file> <warmup_count> <sim_count> <policy> <cache_size>"
  echo "  <cache_size> must be one of: 1, 2, 4"
  echo "  Use 'ALL' as policy to run tests for all available policies."
  exit 1
fi

CONFIG_FILE="champsim_config.json"
TRACE_FILE=$1
WARMUP_COUNT=$2
SIM_COUNT=$3
POLICY=$4
CACHE_SIZE=$5

# Select SETS/WAYS based on cache_size
case "$CACHE_SIZE" in
  1)
    # 1 MB
    SETS=(512  1024 2048 4096  8192  16384)
    WAYS=(32   16   8    4     2      1)
    ;;
  2)
    # 2 MB
    SETS=(1024 2048 4096 8192 16384 32768)
    WAYS=(32   16   8    4     2      1)
    ;;
  4)
    # 4 MB
    SETS=(2048 4096 8192 16384 32768 65536)
    WAYS=(32   16   8    4     2      1)
    ;;
  *)
    echo "Error: invalid cache_size '$CACHE_SIZE'. Use 1, 2 or 4."
    exit 1
    ;;
esac

POLICIES=("$POLICY")
if [[ "$POLICY" == "ALL" ]]; then
  POLICIES=(lru lfu mru tplru arc larc srrip random)
fi


# Helper to update the JSON in-place
#   $1 = jq-filter, rest = any --arg / --argjson flags for jq
update_json() {
  local filter="$1"; shift
  jq "$@" "$filter" "$CONFIG_FILE" > "${CONFIG_FILE}.tmp" \
    && mv "${CONFIG_FILE}.tmp" "$CONFIG_FILE"
}

# 1) Set the replacement policy once


# 2) Sweep through each (sets, ways) pair
for POLICY in "${POLICIES[@]}"; do
  update_json '.LLC.replacement = $pol' --arg pol "$POLICY"
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
done
wait

