dbreplicate: synthetic Gaussian replicas of one correlator configuration

Usage:
  dbreplicate OUTPUT_DB INPUT_DB NUM_SAMPLES FACTOR [SEED]
Example:
  dbreplicate rho.synthetic.sdb output.npt.sdb1 1000 0.01 12345

The two numerical controls are NUM_SAMPLES (positive integer) and FACTOR
(finite nonnegative relative Gaussian standard deviation). For each correlator
key and time, write NUM_SAMPLES complex values:
  C_replica(t) = C_input(t) + FACTOR * abs(C_input(t)) * (Z_re + i Z_im)
where Z_re and Z_im are independent standard normal deviates. Thus FACTOR=0.01
means 1% of the complex magnitude as the noise sigma in EACH component.
Gaussian noise is unbounded; FACTOR is not a maximum deviation. An exactly zero
complex input remains zero. FACTOR=0 makes exact copies, useful for comparison
with dbmerge. No temporal, cross-key, or sample correlations are introduced.

All keys, the original complex centers, and metadata are preserved through the
same StorageCorrFunc API, correlator dispatch, and format selection as dbmerge.
In particular, current SUNClass builds write the modern SUPERB storage format;
other supported correlator types use the backend selected by that API. The
output is directly readable by standard dbutil. This is not a forced legacy
AllConfStoreDB file. A fitter restricted to that legacy API needs its reader
updated to StorageCorrFunc before consuming modern dbmerge output.

Input must contain exactly one configuration, at least one key, finite values,
and a consistent nonzero time extent. Existing output files are never replaced.
Numeric errors and unsupported types fail with a nonzero exit status.
Supported metadata types match dbmerge: hadron1Pt, hadron2Pt, hadron3Pt,
hadronNPartNPtCorr, hadronSUNNPartNPtCorr, hadronSUNClassNPartNPtCorr.
Old SUN keys follow dbmerge's SUNClass conversion behavior.

Default seed is 5489. Supply an optional unsigned 64-bit SEED to get another
sample set. The same input, seed and executable give the same replicas.
Bitwise Gaussian sequences are not promised across standard-library versions.
Work is serial so results do not depend on OMP_NUM_THREADS. Memory holds one
key's NUM_SAMPLES-by-NtCorr array, rather than all correlators' replica arrays.

These are synthetic software-test data, not independent gauge configurations.
The original metadata is copied for dbmerge compatibility; retain a separate
run log identifying the seed, factor and synthetic origin.

Build integration: registered in Makefile.am and CMakeLists.txt. Regenerate
Autotools files normally (autoreconf -fi) when building from source.

Integration test (uses an existing one-configuration correlator database):
  python3 tests/test_dbreplicate.py /path/to/dbreplicate /path/to/dbmerge \
      /path/to/dbutil /path/to/input.sdb /path/to/new-test-directory
The optional tests/dbreplicate_fixture.cc creates a two-key SUNClass fixture
from a one-key input, including zero and purely imaginary samples. The fixture
is artificial and intended only to test iteration/complex-value handling.
