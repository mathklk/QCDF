#include "gr_doa.h"

#include <vector>
#include <algorithm>
#include <numeric>

/*
Eigenvector Direction Finding Method from gnuradio-doa
Computes the Phase Difference "phase(in1) - phase(in0)" between two signals (-Pi, Pi]

https://github.com/samwhiting/gnuradio-doa/blob/2684f235261b2c0d75bb5c5d2ba39d7497cf970a/gr-doa/lib/capon_ccf_impl.cc#L57


MIT License

Copyright (c) 2017 Sam Whiting

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
double gr_doa::_capon_ccf_impl_direction(const gr_complex* in0, const gr_complex* in1, int const d_vector_size) {
    // put in0 and in1 into vectors x1 and x2
    std::vector<gr_complex> x1(in0, in0+d_vector_size);
    std::vector<gr_complex> x2(in1, in1+d_vector_size);

    // find average value of each one
    gr_complex x1_avg = accumulate(x1.begin(), x1.end(), gr_complex(0,0)) / gr_complex(d_vector_size,0);
    gr_complex x2_avg = accumulate(x2.begin(), x2.end(), gr_complex(0,0)) / gr_complex(d_vector_size,0);
    // subtract the average value from each element
    for (int i=0; i<d_vector_size; ++i) { x1[i] -= x1_avg; }
    for (int i=0; i<d_vector_size; ++i) { x2[i] -= x2_avg; }
    // find conjugates of x1 and x2
    std::vector<gr_complex> x1_c, x2_c;
    x1_c.reserve(d_vector_size);
    x2_c.reserve(d_vector_size);
    for (int i=0; i<d_vector_size; ++i) { x1_c.push_back(conj(x1[i])); }
    for (int i=0; i<d_vector_size; ++i) { x2_c.push_back(conj(x2[i])); }
    // multiply vectors together
    std::vector<gr_complex> c11, c12, c22;
    c11.reserve(d_vector_size);
    c12.reserve(d_vector_size);
    c22.reserve(d_vector_size);
    transform(x1.begin(), x1.end(), x1_c.begin(), back_inserter(c11), std::multiplies<gr_complex>());
    transform(x1.begin(), x1.end(), x2_c.begin(), back_inserter(c12), std::multiplies<gr_complex>());
    transform(x2.begin(), x2.end(), x2_c.begin(), back_inserter(c22), std::multiplies<gr_complex>());
    // make covariance matrix
    std::vector<std::vector<gr_complex> > covar(2, std::vector<gr_complex>(2, 0));
    covar[0][0] = accumulate(c11.begin(), c11.end(), gr_complex(0,0)) / gr_complex(d_vector_size,0);
    covar[0][1] = accumulate(c12.begin(), c12.end(), gr_complex(0,0)) / gr_complex(d_vector_size,0);
    covar[1][1] = accumulate(c22.begin(), c22.end(), gr_complex(0,0)) / gr_complex(d_vector_size,0);
    covar[1][0] = conj(covar[0][1]);

    // find eigenvalues
    gr_complex lambda0 = (covar[0][0]+covar[1][1]+sqrt(pow((covar[0][0]+covar[1][1]),2)-gr_complex(4,0)*(covar[0][0]*covar[1][1]-covar[0][1]*covar[1][0])))/gr_complex(2,0);
    gr_complex lambda1 = (covar[0][0]+covar[1][1]-sqrt(pow((covar[0][0]+covar[1][1]),2)-gr_complex(4,0)*(covar[0][0]*covar[1][1]-covar[0][1]*covar[1][0])))/gr_complex(2,0);
    gr_complex max_lambda = (abs(lambda0) > abs(lambda1)) ? abs(lambda0) : abs(lambda1);

    return std::arg((max_lambda - covar[0][0])/covar[0][1]);
}


float gr_doa::phaseDifference(ComplexList const& A, ComplexList const& B, int const start, int const end) {
    if (A.size() != B.size()) {
        qCritical() << "phaseDifference() inA and inB must have the same size" << A.size() << "<->" << B.size();
        return NAN;
    }
    if (start < 0) {
        qCritical() << "phaseDifference() firstIndex must not be negative" << start;
        return NAN;
    }
    if (end < 0) {
        qCritical() << "phaseDifference() lastIndex must not be negative" << end;
        return NAN;
    }
    if (end <= start) {
        qCritical() << "phaseDifference() lastIndex must be larger than firstIndex" << end << "<=" << start;
        return NAN;
    }
    if (start >= A.size()) {
        qCritical() << "phaseDifference() firstIndex must not be larger than input array size" << start << ">=" << A.size();
        return NAN;
    }
    if (end >= A.size()) {
        qCritical() << "phaseDifference() lastIndex must not be larger than input array size" << end << ">=" << A.size();
        return NAN;
    }
    int const size = end - start;
    std::vector<gr_complex> A_(size), B_(size);
    for (int i = start; i < end; ++i) {
        A_[i - start] = A[i];
        B_[i - start] = B[i];
    }
    return _capon_ccf_impl_direction(A_.data(), B_.data(), size);
}

double gr_doa::phaseDiffToDistance(double const dp, double const lambda) {
    return dp * lambda / (2 * M_PI);
}

double gr_doa::angle(double const dp, double const lambda, double const d) {
    //return 180 * qAsin(dp*lambda/(2 * M_PI * d)) / M_PI;
    return 180 * qAsin(phaseDiffToDistance(dp, lambda) / d) / M_PI;
}