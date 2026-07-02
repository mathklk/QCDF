#include "music.h"

#include "gr_music.h"

Spectrum music(
    AntennaArrayType const array,
    float const dLambda,
    QVector<ComplexList> const& collection,
    int& peak,
    double& minY,
    double& maxY
) {
    // Parameters for 3-element ULA
    int num_targets = 1;
    int num_ant = 3;
    int pspectrum_len;
    int angleOffset_deg;

    // Instantiate the selected MUSIC array model
    Music* music = nullptr;
    //QString const selectedArray = ui->comboBoxArrayType->currentText();
    if (array == AntennaArrayType::ULA) {
        pspectrum_len = 180 + 1;  // -90 to +90 inclusive
        angleOffset_deg = -90; // Center spectrum around broadside
        music = new MusicLinArray(dLambda, num_targets, num_ant, pspectrum_len);
    } else if (array == AntennaArrayType::UCA) {
        pspectrum_len = 360; // -180 to +180 inclusive (no +1 bc of wraparound)
        angleOffset_deg = 0;
        music = new MusicTriangle(dLambda, num_targets, pspectrum_len);
    } else {
        qCritical() << "Unsupported array type" << int(array);
    }

    // Copy IQ data from collection into Eigen Matrix
    int N = collection.first().size();
    //qDebug() << "N:" << N << "num_ant:" << num_ant << "collection[0].size():" << collection[0].size();
    Eigen::MatrixXcf samples = Eigen::MatrixXcf::Zero(N, num_ant);
    for (int ant = 0; ant < num_ant; ++ant) {
        auto const& complexList = collection[ant];
        for (int i = 0; i < N; ++i) {
            samples(i, ant) = complexList[i];
        }
    }

    // Compute autocorrelation matrix Rxx (average outer product)
    Eigen::MatrixXcf Rxx = Eigen::MatrixXcf::Zero(num_ant, num_ant);
    for (int i = 0; i < N; ++i) {
        Eigen::VectorXcf x = samples.row(i);
        Rxx += x * x.adjoint();
    }
    Rxx /= static_cast<float>(N);

    // Prepare inputs/outputs for work() (expects flattened matrix)
    gr_vector_const_void_star input_items;
    input_items.push_back(Rxx.data());  // Rxx as input

    Eigen::VectorXf spectrum(pspectrum_len);
    gr_vector_void_star output_items;
    output_items.push_back(spectrum.data());  // Output pseudo-spectrum

    // Process (noutput_items = 1 for one Rxx matrix)
    int noutput_items = 1;
    music->work(noutput_items, input_items, output_items);

    // Clean up
    delete music;

    // spectrum now contains the MUSIC pseudo-spectrum (dB scale)
    // Peaks indicate estimated DOA angles
    Spectrum qSpectrum;
    maxY = -INFINITY;
    minY = INFINITY;
    for (int i = 0; i < spectrum.size(); ++i) {
        int angle = i + angleOffset_deg;
        while (angle >= 180) angle -= 360;
        while (angle < -180) angle += 360;
        qSpectrum.append({angle, spectrum(i)});
        if (spectrum(i) > maxY) {
            maxY = spectrum(i);
            peak = angle;
        }
        minY = qMin(minY, static_cast<double>(spectrum(i)));
    }

    // Sort qSpectrum by angle (.first)
    std::sort(qSpectrum.begin(), qSpectrum.end(), [](const QPair<int,float>& a, const QPair<int,float>& b) {
        return a.first < b.first;
    });
    return qSpectrum;
}
