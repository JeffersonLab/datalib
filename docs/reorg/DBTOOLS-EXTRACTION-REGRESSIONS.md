# Standalone dbtools extraction and regression

The reorg stack now installs all 20 eligible database utilities from datalib.
The nondefault `dbcorr_read` tool also moved. ADAT retains `extract_corr`, its
ENSEM analysis exporter, and the graph integration check. Changes are confined
to `rge/devel-datalib-reorg`; no devel branches or Kanoni work changed.

## Ownership and implementation

The [database tool guide](../database-tools.md) lists the installed commands and
optional tensor modes. Four shared header/source pairs moved from ADAT to
`include/datalib/legacy` and `lib`: the three legacy `hadron_sun_npart_*` models
and `hadron_convert_corr_util`. Their old ADAT copies/build entries are removed;
ADAT and Redstar include the public datalib headers directly. Their existing
`Hadron` symbols and distinct legacy `mom_type` codecs are preserved. No
compatibility headers or stitched graph support were added.

Five tools now use standard complex vectors instead of ENSEM vector arithmetic:
`dbavgcors`, `dbbin`, `dbwick_rescale`, `dbiso_wick_rescale`, and
`db_transform_corrs`. `dbcanon_mom` uses standard vectors for unchanged values.
Other moves remove unused ADAT/ENSEM frontends or replace the four-spin constant
with datalib's identical value. The standalone dispatch has the same five key
families. No ADAT, ENSEM or QDP symbols occur in the installed moved tools.

The preceding `dbbin` loop/argument fix is retained. Validation exposed an
existing `dbconvert_corr_util` failure: its output dictionary was never
registered. The tool now registers converted keys before inserting values.
The reference comparison uses the previous ADAT implementation with that same
small correction, whose source is retained in evidence. Wick normalization
retains the original ascending factorial product within its defined signed-shift
range and rejects invalid exponents; no Numerical Recipes code was copied.
Legacy hard-coded physical coefficients, flavor factors, conjugation and RNG
behavior remain in place.

Changed C++ uses `/Users/admin/.clang-format` (LLVM, two-space indentation,
indented namespaces). The home file is unchanged; ADAT and datalib local configurations match it.
The extraction provenance is in `dbtools-provenance.json` at the repository root.

## Builds and targeted tests

Fresh isolated source snapshots were made from the reorg branches plus the
current working changes, with no installed incremental overlays. Artifacts
are under `evidence/dbtools-extract-20261109-001/install` in the reorg workspace.
Source implementation hashes/contents match the snapshots that were built.
The build uses Apple Clang/C++20, libxml2, OpenMP/libomp and Accelerate, retaining
the matching pinned QDP/QMP, Hadron, serial/MPI SuperbBLAS and PRIMME dependencies
from the previous validated stack.

- Independent datalib core: **6/6** checks passed; the XML-only
  converter built successfully. ADAT, QDP/QMP and external xpath_reader access was denied.
- FILEDB + serial SuperbBLAS datalib: **11/11** checks passed, all 20 tools built.
- FILEDB without SuperbBLAS: **11/11** checks passed, all 20 tools built.
- Fresh ADAT, ColorVec and Redstar builds, checks and installs passed. Their
  existing integration checks passed; ColorVec has no additional configured tests.
- Fresh Chroma built/installed with ADAT and external xpath_reader access denied.
  Both evaluators link libomp; actual OpenMP teams of one and four were observed.
- The nondefault `dbcorr_read` built standalone and read nine populated SUN keys.
- Source distribution includes every moved tool, private header, fixture and
  test, and no generated database payloads. Packaging was performed outside the
  dependency-denying build sandbox because historical evidence documents have
  ADAT-prefixed filenames; actual builds/tests remained sandboxed.

Frozen, nonempty ADAT-generated SUN/SU(2) binary and XML fixtures and converted
SUNClass keys match byte-for-byte. Fifteen before/after utility comparisons
cover all five legacy dispatch families, normalization, canonicalization,
conversion, and all four complex transformation branches. **747 finite complex
values** passed the unchanged matched per-value gate; the maximum absolute
difference was **2.278173318171838e-13** in the transformation arithmetic.
Encoded keys and metadata match exactly. The binning tests also cover seven
exact/partial/oversized cases, invalid sizes, in-place protection and input hashes.

Noise outputs match ADAT for zero and nonzero widths. The two-key, 4096-sample
seeded replica test passed reproducibility, zero-noise/dbmerge agreement,
invalid-input protection and statistics. `dbprop_diag_t0` selects exactly the
same 32 diagonal keys from 64 synthetic time/spin keys, covering all 16 spin
pairs at two times with exact matrix payloads and finite 2x2 readback.
`dbmeta` output matches the previous tool on a real correlator and native
propagator tensor; source hashes remain unchanged.

These synthetic tests validate software behavior and native-double wire
compatibility; they are not independent gauge configurations or fitted physics.
No single-precision compatibility claim is made by this native-double pass.

## Full native-double NPT/CORR_SUPERB regression

The complete existing matrix passed with its original dataset and explicit
DISCO supplement, original policy, existing deferrals, unchanged per-value
numerical gate and memory limits:

- **30 paired cases, 925 keys, 4197 finite complex values**, zero strict failures.
- **3 NPT-only cases, 6 keys, 96 finite complex values**; these do not establish
  CORR support for the deferred channels.
- All **63 numerical output databases are byte-identical** to the previous
  validated direct-storage/STOCH_HADRON-removal reorg checkpoint.

An independent audit checked full finite readback and shapes, coverage, graph
format 4, XML input version 14, matched vector/time/physics flags, fake-data off,
expected unevaluatable-graph accounting and the exactly-zero eta control.
Existing ff_3pt deferrals and glue/tetra/gluonic-insertion CORR deferrals remain.
The disconnected-loop supplement remains explicit and hash-pinned.

This uses cached native-double tensors and zero CG solves, one MPI rank and
one OpenMP worker per process (forced by the regression runner),
`SB_CACHEGB_CPU=1`, the unchanged 8192 MiB RSS cap and 2 GiB planner. The
separate OpenMP team probe verified one and four workers. It does not qualify
on-demand GenProp, new CG solves, MPI scaling, GPU or latest-YLM builds.
Existing historical qualifications remain in force.

## Retained limitations and evidence

`extract_corr` remains a nondefault ADAT program. Both the pre-move baseline and
the reorg source fail its optional compile at the same missing
`ensemFileName(KeyHadronSUNNPartNPtCorr_t)` overload. That preexisting limitation
was not changed or hidden by the extraction. The orphan `dbconvert.cc`, `dbconvert_read.cc`, and `dbconvert_write.cc`
files have no current target and missing local conversion headers; they remain
in ADAT for separate review. No additional routines were removed.

Earlier fixture-driver failures are retained outside Git: the first driver
used a new-format-only readback API on legacy data; another exposed the real
converter failure; the next lacked JSON escaping for multiline metadata.
The corrected driver passed and its results are included here. The first
replica-fixture compile exposed a stale ADAT include, now replaced directly.
The initial strict-sandbox distribution attempt could not read historical
ADAT-named evidence; the ordinary packaging pass succeeded. These attempts
are not labelled as passing application regressions.

[Compact evidence](evidence/dbtools-extract-20261109-001/independent-audit.json)
contains source/build commands and environments, checks and frozen reference
results, full-suite results and an independent audit. Complete local artifacts,
including input/output databases, preserved failures and source snapshots,
remain under the reorg workspace's `evidence/dbtools-extract-20261109-001/`.
Generated databases, binaries and numerical run directories are not added to Git.

## Publication — 2026-11-10

This report and its compact evidence are committed with the datalib extraction.
The matching consumer commits are:

- ADAT: `a934f3307bf68ba670a1043298e158697cecc845`.
- Redstar: `bca8a45c584c7aae588348579ca9af6bc512957e`.

Validation used the exact source hashes recorded in the compact evidence.
Only publication metadata changed after validation; the C++ implementations,
fixtures, numerical gates and results are unchanged. Preexisting submodule
changes and configure backups remain outside these commits.

Final staging retains original whitespace in frozen evidence, the copied home
style file, and an inherited disabled `#if 0` coefficient block. These are
cosmetic diagnostics; validated C++ sources and fixture bytes were preserved.
