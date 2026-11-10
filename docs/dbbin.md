dbbin OUTPUT INPUT BIN_SIZE

Average successive configuration rows independently for every database key.
BIN_SIZE must be a positive integer. The final partial bin is retained and
normalized by its actual number of configurations. Output configuration IDs
are consecutive from zero, matching the existing convention. Metadata and
serialized keys are copied unchanged. Existing key/value codecs are retained.
Input and output must refer to different files. A different existing output
file is still overwritten, as in the original command.

Regression test (synthetic software fixtures; no Kanoni input required):
  Build dbbin normally against its matching ADAT/datalib installation.
  Compile tests/dbbin_fixture.cc using datalib-config --cxxflags,
  --storage-cxxflags, --ldflags, --storage-ldflags, --libs and --storage-libs.
  The fixture uses standard complex vectors, independently of ENSEM arithmetic.
  python3 tests/test_dbbin.py /path/to/dbbin /path/to/dbbin_fixture \
      /path/to/new-test-directory

Checks cover two keys, complex values, exact and partial bins, bin size one,
a bin larger than the input, unchanged encoded keys and metadata, finite full
readback, input hashes, invalid sizes and rejection of in-place output.
Each subprocess has a timeout to catch a non-advancing bin loop.
