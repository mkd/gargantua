#include "types.h"
#include "bitboard.h"

namespace Stockfish {

Bitboard pawn_attacks_bb_white(Square s) {
    Bitboard b = square_bb(s);
    Bitboard attacks = 0;
    if (file_of(s) > FILE_A) attacks |= (b << 7);
    if (file_of(s) < FILE_H) attacks |= (b << 9);
    return attacks;
}

Bitboard pawn_attacks_bb_black(Square s) {
    Bitboard b = square_bb(s);
    Bitboard attacks = 0;
    if (file_of(s) > FILE_A) attacks |= (b >> 9);
    if (file_of(s) < FILE_H) attacks |= (b >> 7);
    return attacks;
}

// // template<> Bitboard pawn_attacks_bb<WHITE>(Square s) { return pawn_attacks_bb_white(s); }
// // template<> Bitboard pawn_attacks_bb<BLACK>(Square s) { return pawn_attacks_bb_black(s); }

Bitboard attacks_bb(PieceType pt, Square s, Bitboard occupied) {
    Bitboard attacks = 0;
    int f = file_of(s);
    int r = rank_of(s);

    if (pt == KNIGHT) {
        static const int moves[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
        for (int i=0; i<8; i++) {
            int nf = f + moves[i][0];
            int nr = r + moves[i][1];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7) {
                attacks |= square_bb(make_square(File(nf), Rank(nr)));
            }
        }
    } else if (pt == KING) {
        static const int moves[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
        for (int i=0; i<8; i++) {
            int nf = f + moves[i][0];
            int nr = r + moves[i][1];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7) {
                attacks |= square_bb(make_square(File(nf), Rank(nr)));
            }
        }
    }

    if (pt == BISHOP || pt == QUEEN) {
        static const int dirs[4][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};
        for (int i=0; i<4; i++) {
            for (int step=1; step<8; step++) {
                int nf = f + dirs[i][0] * step;
                int nr = r + dirs[i][1] * step;
                if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
                Square ns = make_square(File(nf), Rank(nr));
                attacks |= square_bb(ns);
                if (occupied & square_bb(ns)) break;
            }
        }
    }

    if (pt == ROOK || pt == QUEEN) {
        static const int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (int i=0; i<4; i++) {
            for (int step=1; step<8; step++) {
                int nf = f + dirs[i][0] * step;
                int nr = r + dirs[i][1] * step;
                if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
                Square ns = make_square(File(nf), Rank(nr));
                attacks |= square_bb(ns);
                if (occupied & square_bb(ns)) break;
            }
        }
    }

    return attacks;
}


Bitboard attacks_bb(Piece pc, Square s) {
    if (type_of(pc) == PAWN)
        return color_of(pc) == WHITE ? pawn_attacks_bb_white(s) : pawn_attacks_bb_black(s);
    return attacks_bb(type_of(pc), s, 0);
}

} // namespace Stockfish


namespace Stockfish {
Bitboard LineBB[64][64];
Bitboard BetweenBB[64][64];

struct InitBitboards {
    InitBitboards() {
        for (int s1 = 0; s1 < 64; ++s1) {
            for (int s2 = 0; s2 < 64; ++s2) {
                LineBB[s1][s2] = 0;
                BetweenBB[s1][s2] = 0;
                if (s1 == s2) continue;
                
                int f1 = s1 & 7, r1 = s1 >> 3;
                int f2 = s2 & 7, r2 = s2 >> 3;
                
                int df = f2 - f1;
                int dr = r2 - r1;
                
                if (df == 0 || dr == 0 || df == dr || df == -dr) {
                    int step_f = (df > 0) - (df < 0);
                    int step_r = (dr > 0) - (dr < 0);
                    
                    for (int step = 1; step < 8; ++step) {
                        int nf = f1 + step * step_f;
                        int nr = r1 + step * step_r;
                        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
                        LineBB[s1][s2] |= (1ULL << (nr * 8 + nf));
                    }
                    for (int step = -1; step > -8; --step) {
                        int nf = f1 + step * step_f;
                        int nr = r1 + step * step_r;
                        if (nf < 0 || nf > 7 || nr < 0 || nr > 7) break;
                        LineBB[s1][s2] |= (1ULL << (nr * 8 + nf));
                    }
                    LineBB[s1][s2] |= (1ULL << s1);
                    
                    int cur_f = f1 + step_f;
                    int cur_r = r1 + step_r;
                    while (cur_f != f2 || cur_r != r2) {
                        BetweenBB[s1][s2] |= (1ULL << (cur_r * 8 + cur_f));
                        cur_f += step_f;
                        cur_r += step_r;
                    }
                }
            }
        }
    }
} init_bitboards;
} // namespace Stockfish
