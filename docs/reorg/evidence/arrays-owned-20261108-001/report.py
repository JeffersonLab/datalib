import json
from pathlib import Path

reorg = Path.home() / "Downloads/macos-datalib-reorg"
relative = Path("evidence/arrays-owned-20261108-001")
evidence = reorg / relative
audit = json.loads((evidence / "independent-audit.json").read_text())
paired = audit["paired"]
npt_only = audit["npt_only"]
pairs = paired["cases"]
solo = npt_only["cases"]
table = ["| Case | Keys | Finite complex values | Maximum absolute NPT/CORR difference |",
         "| --- | ---: | ---: | ---: |"]
for row in pairs:
    table.append(f"| `{row['case']}` | {row['keys']} | {row['values']} | {row['max_abs_error']:.8g} |")
table = "\n".join(table)
solo_table = "\n".join(f"| `{r['case']}` | {r['keys']} | {r['values']} |" for r in solo)
deferrals = ", ".join(f"`{c}`" for c in paired["deferred"])
npt_identical = sum(r["npt_byte_identical"] for r in pairs)
corr_identical = sum(r["corr_byte_identical"] for r in pairs)
solo_identical = sum(r["npt_byte_identical"] for r in solo)
npt_ref_fails = sum(r["npt_vs_historical"]["failures"] for r in pairs + solo)
corr_ref_fails = sum(r["corr_vs_historical"]["failures"] for r in pairs)
npt_ref_error = max(r["npt_vs_historical"]["max_abs_error"] for r in pairs + solo)
corr_ref_error = max(r["corr_vs_historical"]["max_abs_error"] for r in pairs)
worst = max(pairs, key=lambda r: r["max_abs_error"])
relative_flags = [r["case"] for r in pairs if r["relative_only_diagnostic_flagged"]]
relative_note = ("The relative-only dbutil diagnostic flagged " +
                 ", ".join(f"`{name}`" for name in relative_flags) +
                 ". Each passed the strict per-value gate with its scaled floor.") if relative_flags else ""
zero_paired = [r["case"] for r in pairs if r["npt_zero_channels"] or r["corr_zero_channels"]]
zero_solo = [r["case"] for r in solo if r["npt_zero_channels"]]
zero_solo_text = ("The NPT-only lane had zero-valued channels in " +
                  ", ".join(f"`{name}`" for name in zero_solo) + ".") if zero_solo else "The NPT-only lane had no exactly zero channels."
assert zero_paired == ["eta_eta_3pt"], zero_paired
uneval = []
for lane, rows in (("paired", pairs), ("NPT-only", solo)):
    for r in rows:
        if r["npt_unevaluable_graphs"] or r["prior_npt_unevaluable_graphs"]:
            uneval.append(f"| {lane} | `{r['case']}` | {r['npt_unevaluable_graphs']} | {r['prior_npt_unevaluable_graphs']} |")
uneval_text = ("NPT logged unevaluable graphs in the cases below. The assembled "
               "correlators passed their respective comparisons or readback checks. "
               "Other cases logged zero.\n\n"
               "| Lane | Case | Reorg graphs | Historical graphs |\n"
               "| --- | --- | ---: | ---: |\n" + "\n".join(uneval)) if uneval else "NPT logged zero unevaluable graphs."
source_lines = (evidence / "reorg-sources.log").read_text().splitlines()
source_table = "\n".join(f"| {line.split()[0]} | `{line.split()[1]}` |" for line in source_lines)
diagnostics = audit["compressed_diagnostics"]
doc = f"""# Bundled array headers and full native-double regression

8 November 2026. Datalib now owns its rank-1 through rank-4 array headers
under `<datalib/model/xml_array*.h>` and has no external xpath_reader
configure, compiler, link or installed-header dependency. Fresh ADAT,
ColorVec, Redstar and Chroma builds used that installation.
Kanoni's full cached native-double matrix for
`rge/devel-datalib-reorg` passed all **{paired['passed']} active paired cases**
with **{paired['keys']} keys and {paired['values']:,} finite complex values**.
There were zero strict per-value failures and zero CG solves. The largest
absolute NPT/CORR difference was **{worst['max_abs_error']:.10g}**
in `{worst['case']}`.

## Stack and acceptance

| Component | Reorg commit |
| --- | --- |
{source_table}

All five components were built from clean source checkouts on
`rge/devel-datalib-reorg` into empty build/install directories with Apple
Clang, libc++ and libomp. Datalib configured without `--with-xpath` and
passed four core and five FILEDB checks, with ADAT, QDP/QMP and external
xpath_reader filesystem access denied. Both builds reproduced the existing
frozen formats and the additional rank-1/2/3/4 fixture captured from the old
implementation before extraction. No existing fixture was regenerated.

The four copied headers retain their `XMLArray` namespace, guards, layout
and algorithms. Only internal include paths and trailing whitespace were
changed. The source notices and COPYING.xpath_reader remain in the package;
[array provenance](evidence/arrays-owned-20261108-001/array-provenance.json)
records the upstream commit and hashes. The dependency removal does not
remove libxml2, OpenMP, or the optional FILEDB dependency.

Chroma's QDP array header has pre-existing rank-1 empty-comparison/operator!=
differences. Its two datalib consumers explicitly select QDP's array headers
first, preserving their previous behavior. Installed-header checks and
both include orders with QDP and standalone xpath_reader passed the same
frozen array wire fixtures. No top-level xml_array compatibility headers
were installed by datalib.

Chroma rebuilt with ADAT and external standalone xpath_reader access denied.
ADAT, ColorVec and Redstar rebuilt against the same new datalib installation
and passed `make check`. QDP++/QMP, SuperbBLAS, Hadron, tensor, FILEDB and
PRIMME support installations were reused; those unchanged support libraries
were not rebuilt. ADAT and QDP retain their own XML support dependencies;
they are not datalib dependencies.

[Build commands and return codes](evidence/arrays-owned-20261108-001/build.json),
[owned-header/dependency verification](evidence/arrays-owned-20261108-001/owned-array-verification.json)
and [complete stack verification](evidence/arrays-owned-20261108-001/stack-verification.json)
record the checks. Actual OpenMP teams of one and four were observed from
the fresh Redstar flags. Both evaluators link libomp, with no libgomp or
libstdc++ in either process.

The fresh install prefixes are under `evidence/arrays-owned-20261108-001/install/`.
The installed ADAT probe also confirmed that the 37 removed compatibility
headers and representative legacy aliases remain absent. Every Chroma job
ran through an executor denying ADAT and external standalone xpath_reader
access. The audit separately rechecked the real executable, wrapper, sandbox
profile and datalib archive hashes.

Both evaluators used the matching
format-4 graph generator and explicit `average_1pt_diagrams=false`.
The runner used one OpenMP thread per job and one MPI rank for Chroma.
It verified the pinned native-double `RsdCG=1e-14` tensor dataset and
explicit one-file DISCO supplement against their manifests. It matched
the graph and evaluator settings, including `pion_3pt`'s two vectors.
Fake data was disabled. Kanoni's runner and policy were used unchanged.
The readback exporter was copied into the reorg evidence directory and
migrated to the direct APIs before compilation; Kanoni sources were untouched.

The independent audit rechecked the program and data hashes, exact key
and time coverage, one-configuration shapes, finite complex readback, and
the unchanged per-value gate
`abs(NPT-CORR) <= 1e-9 * max(abs(NPT),abs(CORR)) + 1e-12 * max(reference_global_max,1e-30)`.
{relative_note}

## Direct API checks

ADAT's ENSEM adapters, CG factory classes and NPoint expansion remain in
ADAT. Shared graph and key definitions retain datalib's public `Hadron`
namespace. Generic serializers use argument-dependent lookup to select
the correct ADAT, ENSEM or Hadron overload.

## Resource configuration

The full paired matrix and NPT-only lane used the established
`SB_CACHEGB_CPU=1` reusable-cache cap, unchanged 8 GiB RSS guard and 2 GiB
tensor planner. Physics inputs, thread/rank counts, numerical gate and
user deferrals were unchanged. The preceding checkpoint's retained
memory-limit failure and cache-capped rerun are documented in the
[prior full clean-stack report](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md).

## Paired results

{table}

Only the declared `eta_eta_3pt` control had an exactly zero channel.
It had zero graphs, one key and 16 zero values in each evaluator.
Every positive case had nonempty graph coverage.

## Deferrals and NPT-only coverage

CORR_SUPERB remained deferred for {deferrals}. The `ff_3pt`
case was also deferred for NPT. The three permitted NPT-only cases
passed complete finite readback with {npt_only['keys']} keys and
{npt_only['values']} values.

| NPT-only case | Keys | Finite complex values |
| --- | ---: | ---: |
{solo_table}

{zero_solo_text}

The `fsq_3pt` NPT-only check retains its explicitly authorized legacy
`gpdf_2_0.00.sdb` input; it does not claim newly generated native-double gpdf data.

{uneval_text}

## Historical no_1pt comparison and evidence

Against the saved 3 November experimental results, {npt_identical}/30
paired NPT databases, {corr_identical}/30 CORR_SUPERB databases, and
{solo_identical}/3 NPT-only databases were byte-identical. Applying the
same value gate to the historical readbacks gave {npt_ref_fails} NPT
failures and {corr_ref_fails} CORR failures. The largest differences were
{npt_ref_error:.10g} for NPT and {corr_ref_error:.10g} for CORR_SUPERB.
The comparison used the saved databases and exports.

Against the immediately preceding cache-capped clean-stack run, {audit["versus_previous_reorg"]["paired_npt_identical"]}/30 NPT,
{audit["versus_previous_reorg"]["paired_corr_identical"]}/30 CORR_SUPERB and
{audit["versus_previous_reorg"]["npt_only_identical"]}/3 NPT-only databases were byte-identical.

The recorded `devel` refs and Kanoni status match before and after the
run. The runner and policy hashes also match. The evidence directory
holds the exact commands, outputs, and independent audit. It records
{len(diagnostics)} large diagnostic logs compressed after verification
of their uncompressed SHA-256 checksums.

- [Paired results]({relative}/paired/suite/results.json)
- [NPT-only results]({relative}/npt-only/suite/results.json)
- [Independent audit and per-case historical comparisons]({relative}/independent-audit.json)

All original reorg checkout states also matched their starting snapshots.
The validation run kept the reorg repositories at their recorded extraction
commits and did not change any `devel` ref or Kanoni file.

Coverage is limited to cached tensors on this one-rank build. On-demand
GenProp, multi-rank MPI, GPU and CMake remain outside this run.
"""
(reorg / "ARRAYS-OWNED-REGRESSIONS.md").write_text(doc)
print(reorg / "ARRAYS-OWNED-REGRESSIONS.md")

p = reorg / "README.md"
text = p.read_text()
link = "[Bundled-array regression](ARRAYS-OWNED-REGRESSIONS.md)"
if link not in text:
    text += "\n" + link + " records the complete Kanoni NPT/CORR_SUPERB matrix\nwith the fresh standalone datalib, rebuilt ADAT/ColorVec/Redstar and ADAT-free Chroma.\n"
    p.write_text(text)
