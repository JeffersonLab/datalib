import json
from pathlib import Path

reorg = Path.home() / "Downloads/macos-datalib-reorg"
relative = Path("evidence/native-double-clean-full-20261107-002")
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
doc = f"""# Full native-double regression with the clean datalib stack

7 November 2026. Fresh ADAT, ColorVec and Redstar builds used the same
clean standalone datalib installation as the freshly built ADAT-free Chroma.
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

All five application/library components were built from clean source checkouts
on `rge/devel-datalib-reorg`, using Apple Clang, libc++ and libomp. Datalib
and Chroma are the fresh installations verified in the
[clean-build report](STANDALONE-CLEAN-BUILD.md), with three core and four
FILEDB fixture checks passing. ADAT, ColorVec and Redstar were rebuilt from
new clean checkouts into new empty build/install directories against that
same datalib installation. All three passed `make check`.
Their [build commands and return codes](evidence/native-double-clean-full-20261107-001/build.json)
are retained. Matching QDP++/QMP, SuperbBLAS, Hadron, tensor and PRIMME support
installations were reused; those support libraries were not rebuilt again.

The installed-source commits are shown above. Datalib's later documentation
commit `9f086e1b7f285b196d6956660d1d3884a3e98c50` was pushed before this
regression; the audit confirmed that its code matches the clean installed
source commit `b3b1d42`. No library/application source changes were needed.

The fresh ADAT, ColorVec and Redstar prefixes are under
`evidence/native-double-clean-full-20261107-001/install/`. The shared datalib
and Chroma prefixes are under `evidence/standalone-clean-20261107-001/install/`.
ADAT's installed-header probe confirmed that all 37 removed compatibility
paths are unavailable. Representative old container/I/O aliases were rejected
by negative compilation probes. The fresh Redstar flags produced actual
OpenMP teams of one and four; both evaluators link libomp.

Every Chroma job ran through an executor that denies ADAT filesystem access.
The runner hashes that executor; the independent audit separately checks the
real Chroma executable hash, sandbox profile and datalib archive. Their
hashes match the clean-build verification.

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

## Resource configuration and retained first attempt

The first full attempt passed 29 paired cases and all three NPT-only cases,
but `nucnuc_small` was terminated by the unchanged 8,192 MiB RSS guard at
8,403,888 KiB (8,206.9 MiB). Its results and incomplete Chroma output remain
in `evidence/native-double-clean-full-20261107-001/`; that attempt is not
labelled successful. The supported SuperbBLAS reusable CPU allocation cache
normally defaults to 10% of system RAM, separately from the 2 GiB contraction
planner budget.

A fresh targeted retry passed with `SB_CACHEGB_CPU=1`. The entire paired
matrix and NPT-only lane were then rerun in fresh directories with that same
1 GiB reusable-cache cap. The 8 GiB RSS guard, 2 GiB tensor planner, physics,
thread/rank counts, executables, numerical gate and deferrals were unchanged.
No source fix or larger process-memory allowance was used. The successful
full run is under `evidence/native-double-clean-full-20261107-002/`.
See [resource retry provenance](evidence/native-double-clean-full-20261107-002/resource-retry.json).

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

Against the immediately preceding full direct-API reorg run, {audit["versus_previous_reorg"]["paired_npt_identical"]}/30 NPT,
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
The run did not change any original repository, `devel` ref or Kanoni file.

Coverage is limited to cached tensors on this one-rank build. On-demand
GenProp, multi-rank MPI, GPU and CMake remain outside this run.
"""
(reorg / "NATIVE-DOUBLE-CLEAN-REGRESSIONS.md").write_text(doc)
print(reorg / "NATIVE-DOUBLE-CLEAN-REGRESSIONS.md")

p = reorg / "README.md"
text = p.read_text()
link = "[Full clean-stack regression](NATIVE-DOUBLE-CLEAN-REGRESSIONS.md)"
if link not in text:
    text += "\n" + link + " records the complete Kanoni NPT/CORR_SUPERB matrix\nwith the fresh standalone datalib, rebuilt ADAT/ColorVec/Redstar and ADAT-free Chroma.\n"
    p.write_text(text)
