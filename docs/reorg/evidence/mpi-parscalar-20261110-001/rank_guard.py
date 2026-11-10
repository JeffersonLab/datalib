#!/usr/bin/env python3
"""Guard each real MPI rank process, rather than only the mpirun launcher."""
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import time

limit = int(os.environ.get('REORG_MPI_MAX_RSS_MB', '8192'))
timeout = int(os.environ.get('REORG_MPI_TIMEOUT_SECONDS', '850'))
rank = int(os.environ.get('OMPI_COMM_WORLD_RANK', '0'))
env = dict(os.environ, OMP_NUM_THREADS='2', OMP_DYNAMIC='FALSE')
command = sys.argv[1:]
if not command or limit <= 0 or timeout <= 0:
    raise SystemExit('A command and positive limits are required')
process = None
peak = samples = 0
error = None
start = time.monotonic()


def interrupt(signum, frame):
    raise RuntimeError('MPI launcher interrupted rank: signal '+str(signum))


signal.signal(signal.SIGTERM, interrupt)
signal.signal(signal.SIGINT, interrupt)
try:
    process = subprocess.Popen(command, env=env, start_new_session=True)
    while process.poll() is None:
        if time.monotonic()-start >= timeout:
            raise RuntimeError('Rank timeout exceeded')
        response = subprocess.run(['ps', '-o', 'rss=', '-p', str(process.pid)],
                                  capture_output=True, text=True)
        text = response.stdout.strip()
        if text:
            rss = int(text)
            peak = max(peak, rss)
            samples += 1
            if rss > limit*1024:
                raise RuntimeError('Rank RSS exceeded '+str(limit)+' MiB')
        elif process.poll() is None:
            raise RuntimeError('Unable to monitor real rank RSS: '+response.stderr)
        time.sleep(0.2)
    status = process.returncode
except BaseException as failure:
    error = str(failure)
    if process is not None:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        process.wait()
    status = 125
record = {'rank': rank, 'command': command,
          'child_pid': process.pid if process else None, 'returncode': status,
          'rss_limit_mb': limit, 'peak_rss_kib': peak, 'rss_samples': samples,
          'timeout_seconds': timeout, 'seconds': time.monotonic()-start,
          'omp_num_threads': 2, 'error': error}
Path('resource-rank-'+str(rank)+'.json').write_text(json.dumps(record, indent=2)+'\n')
if error:
    print('MPI rank guard: '+error, file=sys.stderr)
sys.exit(status if status >= 0 else 128-status)
