from collections import Counter
import gzip
import hashlib
import json
import math
from pathlib import Path
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

sys.dont_write_bytecode = True
home = Path.home() / "Downloads"
kanoni = home / "redstar_kanoni.devel"
reorg = home / "macos-datalib-reorg"
evidence = reorg / "evidence/native-double-full-20261106-001"
historical = kanoni / "runs/no1pt-clean-20261103-002/runs"
sys.path.insert(0, str(kanoni / "scripts"))
from compare_correlators import inspect_export
from compare_npt_superb import exported_values


def load(path):
    return json.loads(path.read_text())


def digest(stream):
    sha = hashlib.sha256()
    for data in iter(lambda: stream.read(1024 * 1024), b""):
        sha.update(data)
    return sha.hexdigest()


def file_digest(path):
    with path.open("rb") as stream:
        return digest(stream)


def compare(a, b):
    assert a.keys() == b.keys()
    scale = max(abs(z) for row in a.values() for z in row)
    floor = 1e-12 * max(scale, 1e-30)
    errors = []
    failures = 0
    for key, row in a.items():
        assert len(row) == len(b[key])
        for z, w in zip(row, b[key]):
            assert all(math.isfinite(v) for v in (z.real, z.imag, w.real, w.imag))
            errors.append(abs(z - w))
            failures += errors[-1] > 1e-9 * max(abs(z), abs(w)) + floor
    return {"values": len(errors), "failures": failures, "max_abs_error": max(errors)}


def report_nonevaluable(path):
    stream = path.open() if path.exists() else gzip.open(path.with_suffix(".log.gz"), "rt")
    with stream:
        counts = [int(line.split("=", 1)[1]) for line in stream
                  if line.startswith("Total number of nonevaluatable graphs = ")]
    assert len(counts) == 1, (path, counts)
    return counts[0]


def audit_lane(name, historical_name, paired, expected_cases, expected_deferrals):
    suite = evidence / name / "suite"
    state = load(suite / "results.json")
    old_suite = historical / historical_name
    prior = load(old_suite / "results.json")
    assert (evidence / (name + ".exit")).read_text().strip() == "0"
    assert state["status"] == "passed" and state["requested_chain"] == "no_1pt"
    assert state["npt_only"] == (not paired)
    assert state["data_provenance"] == prior["data_provenance"]
    assert state["disco_provenance"] == prior["disco_provenance"]
    assert len(state["cases"]) == len(expected_cases)
    assert {c["case"] for c in state["cases"]} == expected_cases
    assert {c["case"] for c in state["cases"] if c["status"] == "deferred_by_user"} == expected_deferrals
    for path, sha in {**state["program_sha256"], **state["data_provenance"], **state["disco_provenance"]}.items():
        assert file_digest(Path(path)) == sha, path

    results = []
    for case in state["cases"]:
        assert case["chain"] == "no_1pt"
        label = case["case"]
        if label in expected_deferrals:
            assert not case["commands"] and not (suite / "no_1pt" / label).exists()
            continue
        assert case["status"] == "passed"
        stages = ["graph", "view", "npt"] + (["chroma"] if paired else ["keys", "export"])
        assert [c["stage"] for c in case["commands"]] == stages
        assert all(c["returncode"] == 0 for c in case["commands"])
        current = suite / "no_1pt" / label
        previous = old_suite / "no_1pt" / label
        meta = load(kanoni / "regressions/redstar" / label / "case.json")
        view = (current / "view.log").read_text()
        graph_count = int(re.search(r"Number of graphs = (\d+)", view)[1])
        assert "Version = 4" in view and (graph_count == 0) == (label == "eta_eta_3pt")
        with (current / "corr_graph.bin").open("rb") as graph_file:
            assert int.from_bytes(graph_file.read(4), "big") == 4
        graph_xml = ET.parse(current / "graph.xml")
        npt = ET.parse(current / "npt.xml")
        assert graph_xml.findtext("Param/version") == "14"
        assert graph_xml.findtext("Param/average_1pt_diagrams") == "false"
        assert npt.findtext("ColorVec/Param/fake_data_modeP") == "false"
        assert npt.findtext("Param/average_1pt_diagrams") == "false"

        def read_export(directory, keys_path):
            shapes = inspect_export(directory, keys_path)
            assert len(shapes) == case["keys"] == meta["correlators"]
            assert set(shapes.values()) == {(1, meta["time_samples"])}
            return exported_values(directory)

        if paired:
            compare_dir = current / "comparison"
            npt_dir = compare_dir / "reference"
            a = read_export(npt_dir, compare_dir / "reference-keys.xml")
            b = read_export(compare_dir / "candidate", compare_dir / "candidate-keys.xml")
            strict = compare(a, b)
            assert strict["values"] == case["finite_complex_values"] and strict["failures"] == 0
            assert strict["max_abs_error"] == case["strict_comparison"]["max_abs_error"]
            chroma = ET.parse(current / "chroma.xml").find("Param/InlineMeasurements/elem/Param")
            for tag in ("t_origin", "average_1pt_diagrams", "zeroUnsmearedGraphsP"):
                assert npt.findtext("Param/" + tag) == chroma.findtext(tag), (label, tag)
            assert npt.findtext("ColorVec/Param/num_vecs") == chroma.findtext("num_vecs")
            assert int(chroma.findtext("Nt_forward")) == meta["time_samples"]
            if label == "pion_3pt":
                assert chroma.findtext("num_vecs") == "2"
            assert (current / "chroma.log").read_text().count("CG_SOLVER_TIME:") == case["CG_solves"] == 0
        else:
            npt_dir = current / "npt-export"
            a = read_export(npt_dir, current / "npt-keys.xml")
            b = None
            strict = {"values": sum(map(len, a.values()))}
            assert strict["values"] == case["finite_complex_values"]

        row = {"case": label, "keys": len(a), "graph_count": graph_count, "fake_data": False,
               "npt_unevaluable_graphs": report_nonevaluable(current / "npt.log"),
               "prior_npt_unevaluable_graphs": report_nonevaluable(previous / "npt.log"),
               "npt_zero_channels": sum(all(z == 0 for z in values) for values in a.values()), **strict}
        prior_a = previous / ("comparison/reference" if paired else "npt-export")
        row["npt_byte_identical"] = file_digest(current / "output.sdb1") == file_digest(previous / "output.sdb1")
        row["npt_vs_historical"] = compare(exported_values(prior_a), a)
        if paired:
            row["corr_zero_channels"] = sum(all(z == 0 for z in values) for values in b.values())
            row["CG_solves"] = case["CG_solves"]
            row["corr_byte_identical"] = file_digest(current / "superb.sdb1") == file_digest(previous / "superb.sdb1")
            row["corr_vs_historical"] = compare(exported_values(previous / "comparison/candidate"), b)
            row["relative_only_diagnostic_flagged"] = bool(case["dbutil_relative_only_diagnostic"])
            if label == "eta_eta_3pt":
                assert row["npt_zero_channels"] == row["corr_zero_channels"] == len(a) == 1
        results.append(row)
    return {"passed": len(results), "deferred": sorted(expected_deferrals),
            "keys": sum(c["keys"] for c in results), "values": sum(c["values"] for c in results),
            "cases": results}


def git_snapshot():
    def git(repo, *args):
        return subprocess.check_output(["git", "--no-optional-locks", "-C", str(home / repo), *args], text=True)
    result = git("redstar_kanoni.devel", "status", "--porcelain=v1", "--branch")
    for repo in ("redstar.devel", "adat.devel", "chroma.devel", "colorvec.devel", "redstar_kanoni.devel"):
        result += repo + " " + git(repo, "rev-parse", "refs/heads/devel")
    return result


def compress_large_logs():
    records = []
    for log in sorted(evidence.glob("*/suite/no_1pt/*/*.log")):
        if log.stat().st_size < 64 * 1024 * 1024:
            continue
        archive = log.with_suffix(".log.gz")
        size = log.stat().st_size
        before = file_digest(log)
        with log.open("rb") as src, gzip.open(archive, "xb", compresslevel=3) as dst:
            shutil.copyfileobj(src, dst)
        with gzip.open(archive, "rb") as restored:
            assert digest(restored) == before, archive
        records.append({"archive": str(archive.relative_to(evidence)), "original_bytes": size,
                        "original_sha256": before, "archive_bytes": archive.stat().st_size})
        log.unlink()
    return records


def main():
    policy = load(kanoni / "docs/regression-policy.json")
    policy_path = kanoni / "docs/regression-policy.json"
    recorded_sha = (evidence / "policy-runner-library.sha256").read_text().splitlines()
    for line in recorded_sha:
        sha, path = line.split("  ", 1)
        assert file_digest(kanoni / path) == sha
    (evidence / "devel-after.log").write_text(git_snapshot())
    assert (evidence / "devel-before.log").read_bytes() == (evidence / "devel-after.log").read_bytes()
    inventory = {p.parent.name for p in (kanoni / "regressions/redstar").glob("*/case.json")}
    deferred = {n for n, v in policy["deferred_by_user"].items() if v["corr_superb"]}
    npt_only_cases = {n for n in deferred if not policy["deferred_by_user"][n]["npt"]}
    paired = audit_lane("paired", "native-full", True, inventory, deferred)
    npt_only = audit_lane("npt-only", "native-npt-only", False, npt_only_cases, set())
    assert (paired["passed"], paired["keys"], paired["values"]) == (30, 925, 4197)
    assert (npt_only["passed"], npt_only["keys"], npt_only["values"]) == (3, 6, 96)
    result = {"status": "passed", "paired": paired, "npt_only": npt_only,
              "fully_deferred": sorted(n for n in deferred if policy["deferred_by_user"][n]["npt"]),
              "policy_sha256": file_digest(policy_path), "devel_and_kanoni_status_match": True}
    audit_path = evidence / "independent-audit.json"
    archives = load(audit_path).get("compressed_diagnostics", []) if audit_path.exists() else []
    result["compressed_diagnostics"] = archives + compress_large_logs()
    audit_path.write_text(json.dumps(result, indent=2) + "\n")
    print("Independent readback passed for 30 paired and 3 NPT-only cases.", flush=True)


if __name__ == "__main__":
    main()
