#!/bin/bash

mkdir -p hw2_results

policies=("champsim" "champsim_lip" "champsim_bip" "champsim_dip")

traces=(
  "602.gcc_s-2226B.champsimtrace.xz"
  "621.wrf_s-8065B.champsimtrace.xz"
  "627.cam4_s-490B.champsimtrace.xz"
  "654.roms_s-293B.champsimtrace.xz"
  "657.xz_s-2302B.champsimtrace.xz"
)

run_job()
{
  policy=$1
  trace=$2

  trace_name=${trace%.champsimtrace.xz}

  echo "Starting $policy on $trace_name"

  ./bin/$policy \
    --warmup-instructions 25000000 \
    --simulation-instructions 100000000 \
    traces/$trace \
    > hw2_results/${trace_name}_${policy}.txt

  echo "Finished $policy on $trace_name"
}

export -f run_job

for trace in "${traces[@]}"; do
  for policy in "${policies[@]}"; do

    run_job "$policy" "$trace" &

    while [ "$(jobs -rp | wc -l)" -ge 2 ]; do
      sleep 5
    done

  done
done

wait

echo "All 20 simulations finished."