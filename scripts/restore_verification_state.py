#!/usr/bin/env python3
"""Restore small success receipts; test datasets and binaries are never cached here."""
import argparse
import json
from pathlib import Path
from urllib.request import Request, urlopen

from verification_state import load_state, merge_states


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--offline", action="store_true")
    args = parser.parse_args()
    state = load_state(Path(".verify-helper/verified-state.seed.json"))
    state = merge_states(state, load_state(Path(".verification/saved-state.json")))
    if not args.offline:
        # This is our published, reviewed main state, never a PR-provided URL.
        url = "https://blueberry1001.github.io/Blueberry-library/assets/verification-state.json"
        try:
            with urlopen(Request(url, headers={"Cache-Control": "no-cache"}), timeout=15) as response:
                data = response.read(4 * 1024 * 1024 + 1)
            if len(data) > 4 * 1024 * 1024:
                raise ValueError("published state exceeds size limit")
            temporary = Path(".verification/published-state.json")
            temporary.parent.mkdir(parents=True, exist_ok=True)
            temporary.write_bytes(data)
            state = merge_states(state, load_state(temporary))
        except (OSError, ValueError) as error:
            print(f"Published receipts unavailable ({type(error).__name__}); using saved/seed receipts.")
    destination = Path(".verification/previous-state.json")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(state, ensure_ascii=False, indent=2) + "\n")
    Path(".verification/state.json").write_text(destination.read_text())


if __name__ == "__main__":
    main()
