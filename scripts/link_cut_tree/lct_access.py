import os
import sys
from manim import *

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from link_cut_tree.lct_base import LCTBaseScene
from splay_tree.splay_base import GREEN, RED, YELLOW, MUTED

class LCTAccessScene(LCTBaseScene):
    def construct(self):
        title = self._section_title("LCT Access(v)  —  Dynamic Splay & Splice", color=GREEN)
        self.play(Write(title, run_time=0.4))

        # Absolute Map Layout shifted down by 1.2 units to clear the header
        W = [ 0.0,  1.6, 0]
        P = [-2.0,  0.0, 0];  R = [ 2.0,  0.0, 0]
        V = [-3.5, -1.6, 0];  A = [-0.5, -1.6, 0]; X = [ 2.0, -1.6, 0]
        C = [-4.5, -3.2, 0];  D = [-2.5, -3.2, 0]

        n_w = self._node("w", W);   n_p = self._node("p", P);   n_r = self._node("r", R)
        n_v = self._node("v", V, color=RED); n_a = self._node("a", A); n_x = self._node("x", X)
        n_c = self._node("c", C);   n_d = self._node("d", D)
        all_nodes = VGroup(n_w, n_p, n_r, n_v, n_a, n_x, n_c, n_d)

        # Build live dynamic layout connections
        e_wp = self._dynamic_dashed(n_w, n_p)
        e_wr = self._dynamic_solid(n_w, n_r)
        e_rx = self._dynamic_solid(n_r, n_x)
        e_pv = self._dynamic_solid(n_p, n_v)
        e_pa = self._dynamic_dashed(n_p, n_a)
        e_vc = self._dynamic_solid(n_v, n_c)
        e_vd = self._dynamic_dashed(n_v, n_d)
        all_edges = VGroup(e_wp, e_wr, e_rx, e_pv, e_pa, e_vc, e_vd)

        # Dynamic local path capsules
        b_wrx = self._enclose(n_w, n_r, n_x)
        b_pv  = self._enclose(n_p, n_v)
        b_c   = self._enclose(n_c)
        b_a   = self._enclose(n_a)
        b_d   = self._enclose(n_d)
        all_bounds = VGroup(b_wrx, b_pv, b_c, b_a, b_d)

        self.play(FadeIn(all_bounds), FadeIn(all_edges), FadeIn(all_nodes), run_time=0.8)
        
        hint = self._hint("Goal: Splay v to top of its auxiliary path, then splice cleanly to w", RED, title)
        self.play(FadeIn(hint), self._pulse(n_v))
        self.wait(0.8)

        # ─── STEP 1: PHYSICAL SPLAY ROTATION ───
        lbl1 = self._hint("Step 1: Execute Right Rotation (Zig). v climbs above parent p.", YELLOW, title)
        self.play(ReplacementTransform(hint, lbl1), FadeOut(b_pv))

        # Adjusted rotation targets (shifted down consistently)
        new_v = P
        new_p = [-1.0, -1.6, 0]
        new_a = [ 0.2, -3.2, 0]

        self.play(
            n_v.animate.move_to(new_v),
            n_p.animate.move_to(new_p),
            n_a.animate.move_to(new_a),
            run_time=1.4,
            rate_func=smooth
        )
        b_v_top = self._enclose(n_v, n_p)
        self.play(FadeIn(b_v_top), run_time=0.4)
        self.wait(0.5)

        # ─── STEP 2: SPLICING THE NEW PATH ───
        lbl2 = self._hint("Step 2: Disconnect w's old right child. Splice solid trunk from w down to v.", YELLOW, title)
        self.play(ReplacementTransform(lbl1, lbl2))

        # Transition edge logic from solid to dashed and vice-versa
        e_wr_new = self._dynamic_dashed(n_w, n_r)
        e_wv_new = self._dynamic_solid(n_w, n_v)

        b_wvp = self._enclose(n_w, n_v, n_p)
        b_rx_isolated = self._enclose(n_r, n_x)

        self.play(
            FadeOut(e_wr), FadeOut(e_wp), FadeOut(b_wrx), FadeOut(b_v_top),
            FadeIn(e_wr_new), FadeIn(e_wv_new), FadeIn(b_wvp), FadeIn(b_rx_isolated),
            run_time=1.0
        )
        
        lbl3 = self._hint("✓ Access successful. Continuous preferred path established from root to target.", GREEN, title)
        self.play(ReplacementTransform(lbl2, lbl3))
        self.play(self._pulse(n_v))
        self.wait(1.5)