#!/usr/bin/python3


# This file is part of Gargantua, a UCI chess engine with NNUE evaluation
# derived from Chess0, and inspired by Code Monkey King's bbc-1.4.
#	 
# Copyright (C) 2022 Claudio M. Camacho
#
# Gargantua is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gargantua is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.	If not, see <http://www.gnu.org/licenses/>.

import sys
import chess
import chess.engine



# mix of FEN positions taken from Elometer, and different chess engine tests suites
# See: https://www.chessprogramming.org/Test-Positions
fen_positions = [ "r1bq1rk1/2p1bppp/p1np1n2/4p3/Pp2P3/1BN2N2/1PPP1PPP/R1BQR1K1 w - - 0 1",
                  "8/R7/1p2k3/2p1q1p1/2P1Q3/1P2K1P1/7r/8 w - - 0 1",
                  "5rk1/2b2ppp/pp3n2/2p1p1B1/4P3/2NP4/PPP2PPP/5RK1 w - - 0 1",
                  "8/8/8/1Pk5/8/8/2P3K1/8 w - - 0 1",
                  "r3k2r/5ppp/p3p3/1p1p4/1PpP4/2P1P3/P3KPPP/RR6 w kq - 0 1",
                  "k7/8/3r4/8/4N3/5K2/5P2/8 w - - 0 1",
                  "r2qr2k/6pp/pp1p4/3Pn1N1/8/1P4P1/P2Q3P/R3R1K1 w - - 0 1",
                  "8/2P5/3K4/5b2/1p6/6k1/8/8 w - - 0 1",
                  "R7/5qpk/1p6/2p1rb1p/2Q5/2P3P1/1P3P1P/7K w - - 0 1",
                  "2r3k1/1ppq1pp1/p1n2n1p/8/3P4/1PBQ1N1P/P4PP1/3R2K1 w - - 0 1",
                  "r2qk2r/ppp2ppp/1n1p1nb1/8/2PP4/2NB2P1/PP3PP1/R1BQ1RK1 w kq - 0 1",
                  "2b5/ppp1k1pp/2n1p3/1BNp1p2/3P3P/PP2P1P1/1P1K1P2/8 w - - 0 1",
                  "r2q1rk1/1ppn1ppp/p2np3/3p4/B2P4/2P1PN2/P1P1QPPP/R4RK1 w - - 0 1",
                  "r4rk1/2qnppb1/4p1p1/4P1N1/1p1P1P2/1P6/1KP1N3/3R2QR w - - 0 1",
                  "4br2/p1q1p1k1/4Q1p1/1pN2n2/1P1b4/8/P3B1PP/4BR1K w - - 0 1",
                  "8/p7/Pp1kB3/1Ppn2K1/2P5/8/8/8 w - - 0 1",
                  "rr3nk1/6q1/2p1pRP1/3pP1Qp/6p1/2P3P1/N6P/K1R5 w - - 0 1",
                  "6r1/pkpq3p/6p1/2P5/2NPnP2/1P5R/7K/3Q4 w - - 0 1",
                  "r1bqk2r/pp1n1ppp/2p2n2/3p4/P2Pp3/2P1P3/2PN1PPP/R1BQKB1R w KQkq - 0 1",
                  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                  "7R/8/8/8/3kp3/8/r7/4K3 w - - 0 1",
                  "r4r1k/1p1bbppp/1qp1pn2/4B3/3P3P/1P1B1QR1/P3NPP1/1K1R4 w - - 0 1",
                  "r2qr1k1/1bpn1p1p/1p3bp1/p3p3/P1P5/4PN1P/1PQ1BPPB/R2R2K1 w - - 0 1",
                  "5rk1/R6p/3pp3/2p1n2r/2q5/2B5/1PQ2PPP/5RK1 w - - 0 1",
                  "5q2/1p2k3/p2p4/5bp1/1B6/P2P1P2/2K3P1/1Q6 w - - 0 1",
                  "r4r2/2k2qpp/pRbbR3/P2p1P2/P1pP2P1/2B2Q2/2B5/6K1 w - - 0 1",
                  "r2k3r/pp3pb1/3p3p/1BpP2p1/Q7/P1P2q2/1P3P2/R3R1K1 w - - 0 1",
                  "rn2k2r/1bQ1qpp1/p3p3/2bp4/P2N3p/2PB4/1P3PPP/R1B1R1K1 w kq - 0 1",
                  "3k1r2/4R3/2pB4/pqP2b1p/3P4/2Pp2P1/8/2K1Q3 w - - 0 1",
                  "rr4k1/R1Q2ppp/4pq2/8/7n/3P4/2P2NPP/5R1K w - - 0 1",
                  "r3r1k1/4b1pp/3p3P/6P1/2p1b1q1/4B3/PP1Q2B1/K5RR w - - 0 1",
                  "4r2k/5Qpp/8/2N5/3n2p1/8/2P3PP/4qR1K w - - 0 1",
                  "r2qrnk1/pp2bppp/2p1bn2/3p2B1/3P4/2NBPN2/PPQ2PPP/1R3RK1 w - - 0 1",
                  "r3kb1r/pp4p1/2p1p1p1/4p3/8/2PqPQ1P/PP1N2P1/1R2K2R w Kkq - 0 1",
                  "3kN2b/2p4P/2p2p2/2P2P1K/8/8/8/8 w - - 0 1",
                  "rn2k2r/p4pp1/1p2p2p/1P1pPn1P/2qP4/5N2/2PB1PP1/RQ2K2R w KQkq - 0 1",
                  "6k1/p1P5/P1r5/2p1K3/8/8/2R5/8 w - - 0 1",
                  "r2q1rk1/pbp1bppp/1p2pn2/6B1/3P4/3B1N2/PPP1QPPP/R4RK1 w - - 0 1",
                  "rr4k1/1q3ppp/1bQRp3/6P1/4P3/P4N2/1P6/K3R3 w - - 0 1",
                  "4k2r/1p3ppp/p1n5/2r1p3/4P3/2N5/PP2KPPP/2RR4 w k - 0 1",
                  "8/8/5p2/5p2/5P2/3p3B/5k1P/3K4 w - - 0 1",
                  "r4rk1/pb1qn1pp/1pnNp3/3pPp2/PP1P1P2/5N2/3Q2PP/R3KB1R w KQ - 0 1",
                  "r4r2/6kp/2pqppp1/pbR5/3P4/4QN2/PP3PPP/2R3K1 w - - 0 1",
                  "2r5/pp2kp2/3q1p1Q/3Pp3/6b1/1B6/P1P3PP/1K3R2 w - - 0 1",
                  "r6k/1p4pp/p2q4/1r3P2/2npQ2P/6P1/R1PB1P2/2K1R3 w - - 0 1",
                  "2k4r/p1b2p2/Pp1r1q1p/1Pp1p3/4P1pP/2P1R1P1/5PB1/1RQ3K1 w - - 0 1",
                  "r3k1nr/pp1nq1p1/1bp1b2p/3pPp2/3P1B2/2PB1N2/PP1NQ1PP/R3K2R w KQkq - 0 1",
                  "r1bq1rk1/ppp2pp1/2np1n1p/3Np1NQ/7P/1B1P4/PPP2PP1/2KR3R w - - 0 1",
                  "4r1k1/1b4pp/p1p1r3/2Pp1qb1/PP2p3/2N3B1/4RPPP/3RQ1K1 w - - 0 1",
                  "r4r2/pppqbpk1/2n1b2p/4p1p1/3pP3/3P2PP/PPPN1PBK/R2Q1RN1 w - - 0 1",

                  # https://www.chessprogramming.org/Silent_but_Deadly
                  "1r2r1k1/3bnppp/p2q4/2RPp3/4P3/6P1/2Q1NPBP/2R3K1 w - - 0 1",
                  "6k1/pp2rp1p/2p2bp1/1n1n4/1PN3P1/P2rP2P/R3NPK1/2B2R2 w - - 0 1",
                  "8/2p2kpp/p6r/4Pp2/1P2pPb1/2P3P1/P2B1K1P/4R3 w - - 0 1",
                  "Q5k1/5pp1/5n1p/2b2P2/8/5N1P/5PP1/2q1B1K1 b - - 0 1",
                  "r1b1qrk1/pp4b1/2pRn1pp/5p2/2n2B2/2N2NPP/PPQ1PPB1/5RK1 w - - 0 1",
                  "r1b2rk1/2qnbp1p/p1npp1p1/1p4PQ/4PP2/1NNBB3/PPP4P/R4RK1 w - - 0 1",
                  "r1bq1rk1/ppp2ppp/2n1pn2/3p4/1bPP4/2NBPN2/PP3PPP/R1BQK2R w KQ - 0 1",
                  "r1bqk2r/bp3ppp/p1n1pn2/3p4/1PP5/P1N1PN2/1B3PPP/R2QKB1R b KQkq - 0 1",
                  "r1bqk2r/p2pppbp/2p3pn/2p5/4P3/2P2N2/PP1P1PPP/RNBQR1K1 b kq - 0 1",
                  "r1bqk2r/pp2ppbp/2np1np1/2p5/4P3/1B1P1N1P/PPP2PP1/RNBQK2R w KQkq - 0 1",
                  "r1rn2k1/pp1qppbp/6p1/3pP3/3P4/1P3N1P/PB1Q1PP1/R3R1K1 w - - 0 1",
                  "r2q1rk1/1b1nbpp1/pp2pn1p/8/2BN3B/2N1P3/PP2QPPP/2R2RK1 w - - 0 1",
                  "r2q1rk1/pb2bppp/npp1pn2/3pN3/2PP4/1PB3P1/P2NPPBP/R2Q1RK1 w - - 0 1",
                  "r2q1rk1/pppb1pbp/2np1np1/4p3/2P5/P1NPPNP1/1P3PBP/R1BQK2R w KQ - 0 1",
                  "r2qkb1r/pb1n1p2/2p1p2p/4P1pn/PppP4/2N2NB1/1P2BPPP/R2Q1RK1 w kq - 0 1",
                  "r4r1k/pp1bq1b1/n2p2p1/2pPp1Np/2P4P/P1N1BP2/1P1Q2P1/2KR3R w - - 0 1",
                  "r4rk1/1q2bppp/p1bppn2/8/3BPP2/3B2Q1/1PP1N1PP/4RR1K w - - 0 1",
                  "r4rk1/pp2qpp1/2pRb2p/4P3/2p5/2Q1PN2/PP3PPP/4K2R w K - 0 1",
                  "r7/3rq1kp/2p1bpp1/p1Pnp3/2B4P/PP4P1/1B1RQP2/2R3K1 b - - 0 1",
                  "rn1q1rk1/p4pbp/bp1p1np1/2pP4/8/P1N2NP1/1PQ1PPBP/R1B1K2R w KQ - 0 1",
                  "rnbqr1k1/pp1p1ppp/5n2/3Pb3/1P6/P1N3P1/4NPBP/R1BQK2R w KQ - 0 1",

                  # https://www.chessprogramming.org/CCR_One_Hour_Test
                  "rn1qkb1r/pp2pppp/5n2/3p1b2/3P4/2N1P3/PP3PPP/R1BQKBNR w KQkq - 0 1",
                  "r1bqk2r/ppp2ppp/2n5/4P3/2Bp2n1/5N1P/PP1N1PP1/R2Q1RK1 b kq - 1 10",
                  "rnbq1rk1/pppp1ppp/4pn2/8/1bPP4/P1N5/1PQ1PPPP/R1B1KBNR b KQ - 1 5",
                  "r4rk1/3nppbp/bq1p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - - 1 12",
                  "r1bq1rk1/1pp2pbp/p1np1np1/3Pp3/2P1P3/2N1BP2/PP4PP/R1NQKB1R b KQ - 1 9",
                  "rnbqkb1r/pppp1ppp/5n2/4p3/4PP2/2N5/PPPP2PP/R1BQKBNR b KQkq f3 1 3",
                  "r2q1rk1/2p1bppp/p2p1n2/1p2P3/4P1b1/1nP1BN2/PP3PPP/RN1QR1K1 w - - 1 12",
                  "r1bqkb1r/2pp1ppp/p1n5/1p2p3/3Pn3/1B3N2/PPP2PPP/RNBQ1RK1 b kq - 2 7",
                  "r3kb1r/pp1n1ppp/1q2p3/n2p4/3P1Bb1/2PB1N2/PPQ2PPP/RN2K2R w KQkq - 3 11",
                  "r2qkbnr/ppp1pp1p/3p2p1/3Pn3/4P1b1/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 2 6",

                  # https://www.chessprogramming.org/Eigenmann_Rapid_Engine_Test
                  "r1bqk1r1/1p1p1n2/p1n2pN1/2p1b2Q/2P1Pp2/1PN5/PB4PP/R4RK1 w q - 0 1",
                  "5rk1/pp1b4/4pqp1/2Ppb2p/1P2p3/4Q2P/P3BPP1/1R3R1K b - - 0 1",
                  "1nrq1rk1/p4pp1/bp2pn1p/3p4/2PP1B2/P1PB2N1/4QPPP/1R2R1K1 w - - 0 1",
                  "r3r2k/1bq1nppp/p2b4/1pn1p2P/2p1P1QN/2P1N1P1/PPBB1P1R/2KR4 w - - 0 1",
                  "rqn2rk1/pp2b2p/2n2pp1/1N2p3/5P1N/1PP1B3/4Q1PP/R4RK1 w - - 0 1",
                  "r3k2r/5ppp/3pbb2/qp1Np3/2BnP3/N7/PP1Q1PPP/R3K2R w KQkq - 0 1",
                  "rnb2rk1/pp2q2p/3p4/2pP2p1/2P1Pp2/2N5/PP1QBRPP/R5K1 w - - 0 1",
                  "r1b2rk1/p2nqppp/1ppbpn2/3p4/2P5/1PN1PN2/PBQPBPPP/R4RK1 w - - 0 1",
                  "r1bq1rk1/p4ppp/3p2n1/1PpPp2n/4P2P/P1PB1PP1/2Q1N3/R1B1K2R b KQ - 0 1",
                  "r4rk1/ppp2ppp/1nnb4/8/1P1P3q/PBN1B2P/4bPP1/R2QR1K1 w - - 0 1",

                  # https://www.chessprogramming.org/Kaufman_Test
                  "1rbq1rk1/p1b1nppp/1p2p3/8/1B1pN3/P2B4/1P3PPP/2RQ1R1K w - - 0 1",
                  "r1b1r1k1/1ppn1p1p/3pnqp1/8/p1P1P3/5P2/PbNQNBPP/1R2RB1K w - - 0 1",
                  "2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - 0 1",
                  "3r2k1/ppq2pp1/4p2p/3n3P/3N2P1/2P5/PP2QP2/K2R4 b - - 0 1",
                  "1r2r1k1/p4p1p/6pB/q7/8/3Q2P1/PbP2PKP/1R3R2 w - - 0 1",
                  "r2q1r1k/pb3p1p/2n1p2Q/5p2/8/3B2N1/PP3PPP/R3R1K1 w - - 0 1",
                  "1r1q1rk1/p1p2pbp/2pp1np1/6B1/4P3/2NQ4/PPP2PPP/3R1RK1 w - - 0 1",
                  "rn1q1rk1/1b2bppp/1pn1p3/p2pP3/3P4/P2BBN1P/1P1N1PP1/R2Q1RK1 b - - 0 1",
                  "8/p1ppk1p1/2n2p2/8/4B3/2P1KPP1/1P5P/8 w - - 0 1"
                  ]


# depth of each test: use 15-20 for really strong engines, 5-8 for slow/weak engines
depthlimit = 7


# data to collect throughout the tests
count = 0
nodes = 0
time = 0
nps = 0
nodesTotal = 0
timeTotal = 0


# take the path to the chess engine from the command line's first argument
# --> the engine needs to support the UCI protocol
engine = chess.engine.SimpleEngine.popen_uci(sys.argv[1])


# test each of the positions
for fen in fen_positions:
    count = count + 1

    board = chess.Board(fen)
    info = engine.analyse(board, chess.engine.Limit(depth=depthlimit))

    nodes = info["nodes"]
    nodesTotal += nodes
    time  = info["time"]
    timeTotal += time
    if time <= 0:
        time = 1
    nps   = nodes / time / 1000

    print("Test " + str(count).rjust(2, "0") + "/" + str(len(fen_positions)) + " -- depth=" + str(depthlimit) + "; nodes=" + str(nodes) + "; time=" + str(time) + "s; nps=" + "{:.0f}".format(nps) + " knps", flush=True)


# final results, including total nodes, time and averages
npsAvg = nodesTotal / timeTotal / 1000
print("========================================================================", flush=True)
print("Total nodes=" + str(nodesTotal) + "; time=" + "{:0.2f}".format(timeTotal) + "s", flush=True)
print("Avg. nodes=" + "{:.0f}".format(nodesTotal / count) + "; avg. time=" + "{:0.2f}".format(timeTotal / count) + "s; nps=" + "{:.0f}".format(npsAvg) + " knps", flush=True)


# stop engine before terminating
engine.quit()
