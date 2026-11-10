# Standalone database tools

Datalib owns the database utilities formerly installed from ADAT's
`main/dbtools`. They require C++20, libxml2 and OpenMP. FILEDB support is enabled
by default; its provider remains pinned in `other_libs/filedb`. No ADAT, QDP,
QMP or external xpath_reader installation is required.

The normal FILEDB build installs:

- `dbreplicate`, `dbmerge`, `dbmeta`, `dbcombine`, `dbavg`, `dbavg_disco`,
  `dbavgcors`, `dbbin`, `dbavgsrc`;
- `dbprop_diag_t0`, `dbprop_t0fix`, `dbmeson_t0fix`, `dbcanon_mom`;
- `dbwick_rescale`, `dbiso_wick_rescale`, `dbconvert_corr_util`, `dbgaus_corr`,
  `db_rephase_corrs`, `dbconvert_ops_util`, `db_transform_corrs`.

`dbutil`, `view_corr_graph` and `dump_corr_graph` remain datalib-owned.
`dbconvert_ops_util` is XML-only and is also installed by `--without-filedb`
builds. The nondefault key-listing utility can be built with `make dbcorr_read`.
`extract_corr` remains in ADAT because it exports ENSEM analysis data. Three
historical orphan sources (`dbconvert*.cc`) remain there for separate review;
they have no build target and lack their old local conversion headers.

Configure `--with-superbblas=PREFIX` to enable serial tensor operations in
`dbutil`, `dbmeta`, `dbavg_disco`, `dbprop_t0fix` and `dbmeson_t0fix`. The
FILEDB paths build without this option. This does not enable MPI in datalib;
Chroma continues to select its own QDP/SuperbBLAS communication configuration.
Use one matching compiler/OpenMP/BLAS stack for all linked libraries.

Public legacy key headers live under `<datalib/legacy/>`, including the
`hadron_sun_npart_*` family and `hadron_convert_corr_util.h`. The model and
conversion symbols retain their existing `Hadron` names for source
compatibility. Include these headers directly; ADAT no longer owns copies or
forwarding headers for them. Standard complex vectors preserve the existing
native-double ENSEM wire representation.

Old SUN `mom_type` database keys and current SUNClass `mom_class` keys remain
distinct codecs. `dbcanon_mom` retains its historical per-constituent operation;
it is not a replacement for current joint momentum-class rotation. Restoring
legacy database codecs does not restore obsolete stitched graph formats.

The arithmetic ports preserve the original weights, normalization factors,
complex basis coefficients and conjugation conventions. Wick normalization
uses the same ascending factorial product within the original defined
integer-shift range; negative or oversized exponents are rejected instead of
executing an undefined shift. The preceding `dbbin` loop correction is retained.
The converter now registers its output key dictionary before inserting values,
fixing a preexisting failure in `dbconvert_corr_util`.

The hard-coded basis/phase tables and existing command-line policies remain in
place. No additional analyses or removals were introduced. See each tool's usage
output and [binning](dbbin.md) / [replication](dbreplicate.md) notes.

`make check` includes nonempty ADAT-generated binary/XML codec fixtures,
conversion results, binning/remainder checks, and 15 frozen legacy-tool result
comparisons. The runtime Python checks require `python3` (skip status 77 if
unavailable). These use synthetic software inputs, separately from the full
native-double NPT/CORR_SUPERB regression documented under
[reorganization validation](reorg/README.md).
