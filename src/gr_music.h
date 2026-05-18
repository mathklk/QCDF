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

#ifndef GR_MUSIC_H
#define GR_MUSIC_H

#include <Eigen/Dense>


typedef std::vector<const void *> gr_vector_const_void_star;
typedef std::vector<void*> gr_vector_void_star;
typedef std::complex<float> gr_complex;

class Music {
public:
    virtual ~Music() = default;

    virtual int work(int noutput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items) = 0;
};

class MusicLinArray: public Music
{
private:
    float d_norm_spacing;
    int d_num_targets;
    int d_num_ant_ele;
    int d_pspectrum_len;
    float *d_theta;
    Eigen::VectorXf d_array_loc;
    Eigen::MatrixXcf d_vii_matrix;
    Eigen::MatrixXcf d_vii_matrix_trans;

public:
    MusicLinArray(float norm_spacing, int num_targets, int inputs, int pspectrum_len);
    ~MusicLinArray() override = default;

    void amv(Eigen::VectorXcf& v_ii, Eigen::VectorXf& array_loc, float theta);

    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items) override;
};

class MusicTriangle: public Music
{
private:
    float d_norm_spacing;
    int d_num_targets;
    int d_num_ant_ele;
    int d_pspectrum_len;
    float *d_theta;
    float d_circumradius;
    Eigen::VectorXf d_element_angles;
    Eigen::MatrixXcf d_vii_matrix;
    Eigen::MatrixXcf d_vii_matrix_trans;

public:
    MusicTriangle(float norm_spacing, int num_targets, int pspectrum_len);
    ~MusicTriangle() override = default;

    void amv(Eigen::VectorXcf& v_ii,
             const Eigen::VectorXf& element_angles,
             float circumradius,
             float theta);

    int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items) override;
};

#endif /* GR_MUSIC_H */
