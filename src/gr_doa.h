#ifndef GR_DOA_H
#define GR_DOA_H

#include <complex>

#include "frame.h"

typedef std::complex<float> gr_complex;

namespace gr_doa {

/* Eigenvector Direction Finding Method from gnuradio-doa
 * Computes the Phase Difference "phase(in1) - phase(in0)" between two signals (-Pi, Pi]
 *
 * https://github.com/samwhiting/gnuradio-doa/blob/2684f235261b2c0d75bb5c5d2ba39d7497cf970a/gr-doa/lib/capon_ccf_impl.cc#L57
 */
double _capon_ccf_impl_direction(const gr_complex* in0, const gr_complex* in1, int const d_vector_size);

/* Wrapper around original gnu radio implementation with Qt types, input validation
 * and the possibility to select a range
 */
float phaseDifference(ComplexList const& A, ComplexList const& B, int const start, int const end);

double phaseDiffToDistance(double const dp, double const lambda);

/*
Δφ: Phasendifferenz in Radianten
λ : Wellenlänge des Signals
d : Abstand zwischen Empfängern
d und λ müssen in selber Einheit sein

-> α : Winkel gegenüber der Normalen der Empfänger in Grad
Positive Phasendifferenz -> Positives α -> Signal kommt von rechts
Negative Phasendifferenz -> Negatives α -> Signal kommt von links

               Δφ * λ
  α = asin( ----------- )
             2 * π * d
 */
double angle(double const dp, double const lambda, double const d);

}; // namespace gr_doa

#endif // GR_DOA_H
