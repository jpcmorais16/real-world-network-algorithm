#!/usr/bin/env bash
# ============================================================================
# C++ Example Runner
# ============================================================================
# Demonstrates how to build and run the C++ network simulation tool with
# different parameter configurations.
#
# Usage:
#   chmod +x examples/run_cpp_examples.sh
#   ./examples/run_cpp_examples.sh
#
# Prerequisites:
#   - GCC/Clang with C++11 support
#   - GNU Make
#   - Linux / macOS / WSL
# ============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BINARY="$PROJECT_DIR/network_metrics"

echo "=============================================="
echo "  C++ Network Simulation — Example Runs"
echo "=============================================="
echo ""

# ── Step 1: Build ──────────────────────────────────────────────────────────
echo "[1/5] Building the project ..."
cd "$PROJECT_DIR"
make clean && make
echo "      Build successful: $BINARY"
echo ""

# ── Step 2: Show help ─────────────────────────────────────────────────────
echo "[2/5] Showing help message ..."
echo "----------------------------------------------"
"$BINARY" --help
echo "----------------------------------------------"
echo ""

# ── Step 3: Small network — quick sanity check ────────────────────────────
echo "[3/5] Quick run: N=500, m=3, p=0.5, fp=0.3, 2 simulations"
"$BINARY" -N 500 -m 3 -p 0.5 -fp 0.3 -n 2 -o "$PROJECT_DIR/examples/example_output_small.txt"
echo "      Output → examples/example_output_small.txt"
echo ""

# ── Step 4: Medium network ────────────────────────────────────────────────
echo "[4/5] Medium run: N=5000, m=4, p=0.7, fp=0.2, 3 simulations"
"$BINARY" -N 5000 -m 4 -p 0.7 -fp 0.2 -n 3 -o "$PROJECT_DIR/examples/example_output_medium.txt"
echo "      Output → examples/example_output_medium.txt"
echo ""

# ── Step 5: Checkpoint demo ──────────────────────────────────────────────
echo "[5/5] Checkpoint demo: N=2000, m=5, p=0.5, fp=0.5, with -id"
"$BINARY" -N 2000 -m 5 -p 0.5 -fp 0.5 -n 2 -o "$PROJECT_DIR/examples/example_output_checkpoint.txt" -id example_run
echo "      Output → examples/example_output_checkpoint.txt"
echo "      Checkpoint files: checkpoint_example_run_*"
echo ""

# ── Cleanup checkpoint files ──────────────────────────────────────────────
rm -f "$PROJECT_DIR"/checkpoint_example_run_*.txt

echo "=============================================="
echo "  All examples completed successfully!"
echo "=============================================="
