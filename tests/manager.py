#!/usr/bin/env python3
import argparse
import subprocess
import json
import re
import sys
import os
import shutil

# Configuration
ENGINE_NAME = "gargantua"
SRC_DIR = "../src" 
BOOKS_DIR = "./books"
DEFAULT_TC = "10+0.1"
DEFAULT_GAMES = 20

def check_dependencies():
    """Checks if cutechess-cli is available."""
    # Add user provided path to environment
    user_path = "/Applications/CuteChess/build"
    if os.path.exists(user_path):
        os.environ["PATH"] += os.pathsep + user_path

    if not shutil.which("cutechess-cli"):
        print("Error: 'cutechess-cli' not found in PATH.")
        print(f"Checked common locations and {user_path}")
        print("Please install it or download a binary and add it to your PATH.")
        sys.exit(1)

def compile_engine(output_name, defines=[]):
    """Compiles the engine."""
    cmd = ["make", "clean"]
    subprocess.run(cmd, cwd=SRC_DIR, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    cmd = ["make", "-j8", "gargantua"]
    print(f"Compiling {output_name}...", end="", flush=True)
    result = subprocess.run(cmd, cwd=SRC_DIR, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    
    if result.returncode != 0:
        print(" FAILED")
        print(result.stderr.decode())
        sys.exit(1)
        
    shutil.copy(os.path.join(SRC_DIR, "gargantua"), output_name)
    
    # Ad-hoc sign the binary on macOS (needed for ARM64)
    if sys.platform == "darwin":
        print(f"Signing {output_name}...", end="", flush=True)
        subprocess.run(["codesign", "-s", "-", output_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print(" Done")
    
    print(" Done")

def run_match(engine1, engine2, games, tc, sprt=False, concurrency=1, book=None):
    """Runs a match between two engines."""
    name1 = os.path.basename(engine1)
    name2 = os.path.basename(engine2)
    
    print(f"Starting match: {name1} vs {name2}")
    print(f"Games: {games}, TC: {tc}, Threads: {concurrency}")
    
    cmd = [
        "cutechess-cli",
        "-engine", f"cmd={engine1}", f"name={name1}", "dir=.",
        "-engine", f"cmd={engine2}", f"name={name2}", "dir=.",
        "-each", f"proto=uci", f"tc={tc}",
        "-concurrency", str(concurrency),
        "-games", str(games),
        "-repeat"
    ]
    
    if book:
         cmd.extend(["-openings", f"file={book}", "format=pgn", "order=random"])

    if sprt:
        cmd.extend(["-sprt", "elo0=0", "elo1=10", "alpha=0.05", "beta=0.05"])

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)

    # Regex for parsing output
    # cutechess output: Score of name1 vs name2: ...
    score_re = re.compile(rf"Score of {re.escape(name1)} vs {re.escape(name2)}: (\d+) - (\d+) - (\d+)  \[(.*)\] (\d+)")
    
    wins, losses, draws = 0, 0, 0

    try:
        while True:
            line = process.stdout.readline()
            if not line:
                break
            
            if line.startswith("Score of"):
                # Parse score to track who is winning
                match = score_re.search(line)
                if match:
                    wins = int(match.group(1))
                    losses = int(match.group(2))
                    draws = int(match.group(3))
                    
                    total = wins + losses + draws
                    score1 = wins + 0.5 * draws
                    score2 = losses + 0.5 * draws
                    
                    leader = "Tie"
                    diff = 0.0
                    if score1 > score2:
                        leader = name1
                        diff = score1 - score2
                    elif score2 > score1:
                        leader = name2
                        diff = score2 - score1
                    
                    # Overwrite the line with a cleaner status
                    msg = f"\rScore: +{wins} -{losses} ={draws} | Leader: {leader} (+{diff}) | Games: {total}"
                    sys.stdout.write(msg.ljust(80)) # Pad to clear previous line
                    sys.stdout.flush()
                else:
                    sys.stdout.write("\r" + line.strip())
                    sys.stdout.flush()

            elif line.startswith("Started"):
                 # Keep start messages on the same rolling line to avoid spam
                 sys.stdout.write("\r" + line.strip().ljust(80))
                 sys.stdout.flush()
                 
            elif "Elo difference" in line:
                print("\n" + line.strip())
                
            elif "Finished game" in line:
                # Print finished game details on a new line so they are visible
                print("\n" + line.strip())
                
            elif "Disconnect" in line or "stall" in line or "connection" in line:
                 print("\n" + line.strip())
                 
            elif "Finished match" in line:
                print("\n" + line.strip())
                
                # Announce winner
                print("\n" + "="*40)
                if wins > losses:
                    print(f"🏆 WINNER: {name1.upper()} (Score: {wins + 0.5*draws} - {losses + 0.5*draws})")
                elif losses > wins:
                    print(f"🏆 WINNER: {name2.upper()} (Score: {losses + 0.5*draws} - {wins + 0.5*draws})")
                else:
                    print(f"🤝 RESULT: DRAW (Score: {wins + 0.5*draws} - {losses + 0.5*draws})")
                print("="*40 + "\n")

    except KeyboardInterrupt:
        process.kill()
        print("\nMatch cancelled.")

    process.wait()


def main():
    parser = argparse.ArgumentParser(description="Gargantua Testing Framework")
    parser.add_argument("--match", action="store_true", help="Run a match")
    parser.add_argument("--base", type=str, help="Path to base engine executable")
    parser.add_argument("--new", type=str, help="Path to new engine executable (default: compiles from src)")
    parser.add_argument("--games", type=int, default=DEFAULT_GAMES, help="Number of games")
    parser.add_argument("--tc", type=str, default=DEFAULT_TC, help="Time control (e.g., 10+0.1)")
    parser.add_argument("--concurrency", type=int, default=4, help="Concurrency (threads)")
    parser.add_argument("--sprt", action="store_true", help="Use SPRT")
    
    args = parser.parse_args()

    check_dependencies()

    if args.match:
        new_engine = args.new
        if not new_engine:
            compile_engine("./gargantua-new")
            new_engine = "./gargantua-new"
            
        base_engine = args.base
        if not base_engine:
            print("Error: --base argument is required (path to base engine executable).")
            sys.exit(1)

        book = None
        # Check for any pgn file in books dir
        if os.path.exists(BOOKS_DIR):
             for file in os.listdir(BOOKS_DIR):
                 if file.endswith(".pgn") or file.endswith(".epd"):
                     book = os.path.join(BOOKS_DIR, file)
                     break
            
        run_match(new_engine, base_engine, args.games, args.tc, args.sprt, args.concurrency, book)

if __name__ == "__main__":
    main()
