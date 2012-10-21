Simplex algorithm
==========

# Definition of the algorithm #

In an abstract sense, this algorithm works by moving from one Basic Feasible
Solution (BFS) on an *n*-dimensional polytope to another.  This is pretty
abstract for actual code, however, so we'll work from the following pseudocode
as presented in MATH 417, Mathematical Programming.

Pseudocode
----------

## Main pseudocode ##
The goal is to achieve max *c' x*, on a system *A x = b*, x ≥ **0**.

1. If *c j ≤ 0* for all *j*, then the BFS is optimal and stop.
2. Choose some variable/column with *c j > 0*, e.g. *c s* = max { *c j : c j >
   0*, *j* } (Dantzig's rule; this is not optimal, but it can be replaced by a
   function pointer in the actual code.)  If this variable is unbounded,
   stop.  The particular variable can be determined to be unbounded if all of
   its constraints have non-positive coefficients.  In that case, use this *c s*
   to pivot, by finding the lowest positive constraint.
3. Pivot by first determining *t*, the value which the constraint is being
   improved to.  This is determined by *t ≤ b i / a is* for all *i*, and *t* is
   the minimum of these constraints. The lowest *a is* is the constraint which
   is being pivoted upon, and once that is accomplished (see *Pivot pseudocode*
   for that) (note that if there is more than one possible constraint to pivot
   upon, one should be selected.  There are various ways to do that.)
4. Replace *x r* by *x s* in the basis, reëstablish canonical form by the
   pivoting on *a rs*, and then repeat from step 1.

## Pivot pseudocode ##
This is effectively a modified Gaussian elimination.

1. Multiply row/constraint *r* by *1 / a rs* so coeff. of *x s* is 1.
2. Subtract multiples of that row to create zeroes everywhere else in columns
   (i.e. subtract *a is* x new normalised row *r* from each row, or subtract *a
   is / a rs* x nonnormalised row *r* from each row.)  For the objective
   function, subtract *c s / a rs* x old row r from objective function;
   N.B. *b i (new) = b i - (a is)/(a rs) b r ≥ 0* (since *b r/a rs* is
   minimal).  This means the new objective value is *z0 + (c s)/(a rs)b r ≥
   z0*.


# Licensure and implementation #

I'm going to license this under the GNU GPLv3, as is stored in gzipped format in
the repository; right now, I'm planning on writing it in C and possibly using
the [GSL (GNU Scientific Library](http://www.gnu.org/software/gsl/).  I may also
write a separate implementation for GNU Octave.

Considerations for this implementation
----------

Initially, this was supposed to be a modular implementation, into which any
number of independent pivot rules could be inserted.  However, after a bit more
consideration about how different pivot rules work, I decided that the pivot
rules should simply be included in the main body; they can be surrounded by
comment blocks if vastly different pivot rules are to be included in the
function, otherwise I may just write additional separate functions to fit the
diffferent rules.

General implementation
----------

simplex:

## Inputs ##
- vector: objective function (size: n x 1, if using a language that cares about
  sizes.)
- matrix: constraints (size: m x n)
- vector: bfs, basic feasible solution. First m values are nonzero, the rest are
  zero. (size: n x 1)

## Outputs ##
- the output is the best solution; the difficult part of this is the vector
  itself, since the original algorithm involves rearranging variables to
  maintain the canonical form.
- 

