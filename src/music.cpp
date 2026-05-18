#include "music.h"

#include "gr_doa.h"
#include "gr_music.h"

Spectrum music(
    QString const& array,
    float const dLambda,
    QVector<ComplexList> const& collection,
    QPair<int, int> cali,
    QPair<int, int> pong,
    int& peak,
    double& minY,
    double& maxY
    ) {
    QVector<float> phaseOffsets;
    phaseOffsets << 0;
    phaseOffsets << gr_doa::phaseDifference(collection[0], collection[1], cali.first, cali.second);
    phaseOffsets << gr_doa::phaseDifference(collection[0], collection[2], cali.first, cali.second);

    // Parameters for 3-element ULA
    //float D = ui->comboBoxDLambda->currentData().toFloat();  // Normalized spacing (e.g., d/λ)
    int num_targets = 1;
    int num_ant = 3;
    int pspectrum_len;
    int angleOffset_deg;

    // Instantiate the selected MUSIC array model
    Music* music = nullptr;
    //QString const selectedArray = ui->comboBoxArrayType->currentText();
    if (array == "ULA") {
        pspectrum_len = 180 + 1;  // -90 to +90 inclusive
        angleOffset_deg = -90; // Center spectrum around broadside
        music = new MusicLinArray(dLambda, 1, 3, pspectrum_len);
    } else if (array == "UCA") {
        pspectrum_len = 360; // -180 to +180 inclusive (no +1 bc of wraparound)
        angleOffset_deg = 0;
        music = new MusicTriangle(dLambda, num_targets, pspectrum_len);
    }


    // Simulate IQ samples: N snapshots, each with 3 complex samples (one per antenna)
    // int N = 100;  // Number of snapshots (increase for better estimation)
    // Eigen::MatrixXcf samples = Eigen::MatrixXcf::Random(N, num_ant);  // Replace with real IQ data

    // Real IQ Data from Collection
    int N = collection.first().size() / sizeof(IQ);
    Eigen::MatrixXcf samples = Eigen::MatrixXcf::Zero(N, num_ant);
    for (int ant = 0; ant < num_ant; ++ant) {
        auto const& complexList = collection[ant];
        for (int i = pong.first; i < pong.second; ++i) {
            samples(i - pong.first, ant) = complexList[i] * std::polar(1.0f, -phaseOffsets[ant]);
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

void normalizeSpectrum(Spectrum& spectrum, float minY) {
    if (minY == INFINITY) {
        for (auto const& [angle, amp] : spectrum) {
            minY = qMin(minY, amp);
        }
    }
    float const factor = qAbs(minY);
    for (auto& [angle, amp] : spectrum) {
        amp /= factor;
    }
}