/* -*- c++ -*- */
/*
 * https://github.com/EttusResearch/gr-doa/blob/main/lib/MUSIC_lin_array_impl.cc
 *
 * Copyright 2016
 * Srikanth Pagadarai <srikanth.pagadarai@gmail.com>
 * Travis F. Collins <travisfcollins@gmail.com>
 * Copyright 2021
 * Marcin Wachowiak <marcin.r.wachowiak@gmail.com>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "gr_music.h"

#include <QDebug>
#include <cmath>
#include <QVector>

// Make Eigen::VectorXf debuggable with QDebug
QDebug& operator<<(QDebug& dbg, const Eigen::VectorXf& vec)
{
    dbg.nospace() << "[";
    for (int i = 0; i < vec.size(); ++i) {
        dbg.nospace() << vec[i];
        if (i != vec.size() - 1) {
            dbg.nospace() << ", ";
        }
    }
    dbg.nospace() << "]";
    return dbg.space();
}

MusicLinArray::MusicLinArray(
    float norm_spacing,
    int num_targets,
    int inputs,
    int pspectrum_len
):
    d_norm_spacing(norm_spacing),
    d_num_targets(num_targets),
    d_num_ant_ele(inputs),
    d_pspectrum_len(pspectrum_len)
{
    // form antenna array locations centered around zero and normalize
    d_array_loc = Eigen::VectorXf::Zero(d_num_ant_ele);
    //consider replacing with linspace eigen builtin
    for (int nn = 0; nn < d_num_ant_ele; nn++)
    {
        d_array_loc(nn) = d_norm_spacing*0.5*(2*nn-d_num_ant_ele+1);
    }
    // qDebug() << "d_array_loc" << d_array_loc.transpose();
    // form theta vector

    QVector<float> d_theta(d_pspectrum_len);
    d_theta[0] = 0.0;
    float theta_prev = 0.0, theta;
    for (int ii = 1; ii < d_pspectrum_len; ii++)
    {
        theta = theta_prev+180.0/(d_pspectrum_len-1);
        theta_prev = theta;
        d_theta[ii] =EIGEN_PI*theta/180.0;
    }
    // qDebug() << "d_theta" << d_theta;
    // form array response matrix


    Eigen::VectorXcf vii_temp = Eigen::VectorXcf::Zero(d_num_ant_ele);
    d_vii_matrix = Eigen::MatrixXcf(d_num_ant_ele,d_pspectrum_len);
    d_vii_matrix_trans = Eigen::MatrixXcf(d_pspectrum_len,d_num_ant_ele);
    for (int ii = 0; ii < d_pspectrum_len; ii++)
    {
        // generate array manifold vector for each theta
        amv(vii_temp, d_array_loc, d_theta[ii]);
        // add as column to matrix
        d_vii_matrix.col(ii) = vii_temp;
    }
    // save transposed copy
    d_vii_matrix_trans = d_vii_matrix.adjoint();
}

// array manifold vector generating function
void MusicLinArray::amv(Eigen::VectorXcf& v_ii, Eigen::VectorXf& array_loc, float theta)
{
    // sqrt(-1)
    const gr_complex i = gr_complex(0.0, 1.0);
    // array manifold vector
    v_ii = (i*(-1.0*2*EIGEN_PI*cos(theta)*array_loc)).array().exp();
}

int
MusicLinArray::work(int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items)
{
    const gr_complex *in = (const gr_complex *) input_items[0];
    float *out = (float *) output_items[0];

    // process each input vector (Rxx matrix)
    Eigen::MatrixXcf eig_vec(d_num_ant_ele,d_num_ant_ele);
    Eigen::MatrixXcf U_N(d_num_ant_ele,d_num_ant_ele-d_num_targets);
    Eigen::MatrixXcf U_N_sq(d_num_ant_ele-d_num_targets,d_num_ant_ele);
    Eigen::VectorXf out_vec_buf(d_pspectrum_len);

    for (int item = 0; item < noutput_items; item++)
    {
        // make input pointer into matrix pointer
        Eigen::Map<Eigen::MatrixXcf> in_matrix((gr_complex *)in+item*d_num_ant_ele*d_num_ant_ele, d_num_ant_ele, d_num_ant_ele);

        Eigen::Map<Eigen::VectorXf> out_vec(out+item*d_pspectrum_len, d_pspectrum_len);

        // determine EVD of the auto-correlation matrix
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcf> eigensolver(in_matrix);
        eig_vec = eigensolver.eigenvectors();

        // noise subspace and its square matrix
        U_N = eig_vec.leftCols(d_num_ant_ele-d_num_targets);
        U_N_sq = U_N*U_N.adjoint();

        // determine pseudo-spectrum for each value of theta in [0.0, 180.0)
        for (int ii = 0; ii < d_pspectrum_len; ii++)
        {
            out_vec_buf(ii) = 1.0/(d_vii_matrix_trans.row(ii)*U_N_sq*d_vii_matrix.col(ii)).value().real();
        }
        out_vec = 10.0*log10((out_vec_buf/out_vec_buf.maxCoeff()).array());

    }
    // Tell runtime system how many output items we produced.
    return noutput_items;
}

MusicTriangle::MusicTriangle(
    float norm_spacing,
    int num_targets,
    int pspectrum_len
    ):
    d_norm_spacing(norm_spacing),
    d_num_targets(num_targets),
    d_num_ant_ele(3),
    d_pspectrum_len(pspectrum_len)
{
    // Arrange three antennas as an equilateral triangle centered at the origin.
    // For an equilateral triangle, side length s and circumradius R satisfy
    // s = sqrt(3) * R, so R = s / sqrt(3).
    d_circumradius = d_norm_spacing / std::sqrt(3.0f);

    // Element angular positions around the triangle vertices in the xy-plane.
    //     A
    //    / \
    //   C - B
    d_element_angles = Eigen::VectorXf(d_num_ant_ele);
    d_element_angles(0) = 0.0f;
    d_element_angles(1) = 2.0f * static_cast<float>(EIGEN_PI) / 3.0f;
    d_element_angles(2) = 4.0f * static_cast<float>(EIGEN_PI) / 3.0f;
    // qDebug() << "d_element_angles" << d_element_angles;

    // Form theta (azimuth) vector covering [0, 360) degrees.
    QVector<float> d_theta(d_pspectrum_len);
    d_theta[0] = 0.0f;
    float theta_prev = 0.0f;
    for (int ii = 1; ii < d_pspectrum_len; ii++)
    {
        float theta_deg = theta_prev + 360.0f / d_pspectrum_len;
        theta_prev = theta_deg;
        d_theta[ii] = static_cast<float>(EIGEN_PI) * theta_deg / 180.0f;
    }
    // qDebug() << "d_theta" << d_theta;

    // Form array response matrix for the triangle.
    Eigen::VectorXcf vii_temp = Eigen::VectorXcf::Zero(d_num_ant_ele);
    d_vii_matrix = Eigen::MatrixXcf(d_num_ant_ele, d_pspectrum_len);
    d_vii_matrix_trans = Eigen::MatrixXcf(d_pspectrum_len, d_num_ant_ele);
    for (int ii = 0; ii < d_pspectrum_len; ii++)
    {
        // generate array manifold vector for each azimuth angle
        amv(vii_temp, d_element_angles, d_circumradius, d_theta[ii]);
        // add as column to matrix
        d_vii_matrix.col(ii) = vii_temp;
    }
    // save transposed (conjugate) copy
    d_vii_matrix_trans = d_vii_matrix.adjoint();
}

// array manifold vector generating function for the triangle
void MusicTriangle::amv(Eigen::VectorXcf& v_ii,
                        const Eigen::VectorXf& element_angles,
                        float circumradius,
                        float theta)
{
    // sqrt(-1)
    const gr_complex i = gr_complex(0.0, 1.0);

    // Use an equilateral triangle centered at the origin, with the first
    // vertex on the +x axis and the other two spaced 120 degrees apart.
    // For a plane wave from azimuth theta, the phase at each vertex is
    // proportional to the projection onto the propagation direction.
    // Using the same sign convention as the linear array implementation:
    //   a_n(theta) = exp(-j * 2*pi * R * cos(theta - phi_n))

    Eigen::VectorXf proj = (theta - element_angles.array()).cos().matrix();

    v_ii = (i * (-1.0f * 2.0f * static_cast<float>(EIGEN_PI) * circumradius * proj))
               .array()
               .exp();
}

int
MusicTriangle::work(int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items)
{
    const gr_complex *in = (const gr_complex *) input_items[0];
    float *out = (float *) output_items[0];

    // process each input vector (Rxx matrix)
    Eigen::MatrixXcf eig_vec(d_num_ant_ele, d_num_ant_ele);
    Eigen::MatrixXcf U_N(d_num_ant_ele, d_num_ant_ele - d_num_targets);
    Eigen::MatrixXcf U_N_sq(d_num_ant_ele - d_num_targets, d_num_ant_ele);
    Eigen::VectorXf out_vec_buf(d_pspectrum_len);

    for (int item = 0; item < noutput_items; item++)
    {
        // make input pointer into matrix pointer
        Eigen::Map<Eigen::MatrixXcf> in_matrix(
            (gr_complex *)in + item * d_num_ant_ele * d_num_ant_ele,
            d_num_ant_ele,
            d_num_ant_ele);

        Eigen::Map<Eigen::VectorXf> out_vec(
            out + item * d_pspectrum_len,
            d_pspectrum_len);

        // determine EVD of the auto-correlation matrix
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcf> eigensolver(in_matrix);
        eig_vec = eigensolver.eigenvectors();

        // noise subspace and its square matrix
        U_N = eig_vec.leftCols(d_num_ant_ele - d_num_targets);
        U_N_sq = U_N * U_N.adjoint();

        // determine pseudo-spectrum for each azimuth in [0.0, 360.0)
        for (int ii = 0; ii < d_pspectrum_len; ii++)
        {
            out_vec_buf(ii) =
                1.0f /
                (d_vii_matrix_trans.row(ii) * U_N_sq * d_vii_matrix.col(ii))
                    .value()
                    .real();
        }
        out_vec = 10.0f *
                  (out_vec_buf / out_vec_buf.maxCoeff()).array().log10();
    }
    // Tell runtime system how many output items we produced.
    return noutput_items;
}
