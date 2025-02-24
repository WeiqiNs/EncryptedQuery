#include "exp.hpp"

void ipe_total_single_filter_time(const int round){
    // Open the output files.
    std::ofstream file("total_single_filter_time.txt", std::ios_base::app);
    file << "IPE Timings" << std::endl;

    // Generate pp and msk.
    auto pp = IpeFilter::pp_gen(1, 20);
    auto msk = IpeFilter::msk_gen(pp);

    for (int num_col = 1; num_col <= 20; ++num_col){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            auto y = Helper::rand_int_mat(20, 1, 1, std::numeric_limits<int>::max());

            // Set the unselected portion to zero.
            for (int j = num_col + 1; j < 20; ++j){ y[num_col][0] = 0; }

            // Total timings.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = IpeFilter::keygen(pp, msk, y);
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Compute ciphertext.
            auto ct = IpeFilter::enc(pp, msk, x);

            // Decryption time.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = IpeFilter::dec(ct, sk);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;
    }

    // Close the BP.
    BP::close();

    // Create some blank spaces.
    file << std::endl << std::endl;
}

void sse_total_single_filter_time(const int round){
    // Open the output files.
    std::ofstream file("total_single_filter_time.txt", std::ios_base::app);
    file << "SSE Timings" << std::endl;

    // Generate pp and msk.
    auto msk = SseFilter::msk_gen();

    for (int num_col = 1; num_col <= 20; ++num_col){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            auto y = Helper::rand_int_vec(num_col, 1, std::numeric_limits<int>::max());

            // Total timings.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = SseFilter::keygen(msk, y, static_cast<int>(std::pow(2, 20)));
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Compute ciphertext.
            auto ct = SseFilter::enc(msk, x);

            // Decryption time.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = SseFilter::dec(ct, sk);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;
    }
    // Create some blank spaces.
    file << std::endl << std::endl;
}

void our_total_single_filter_time(const int round){
    // Open the output files.
    std::ofstream file("total_single_filter_time.txt", std::ios_base::app);
    file << "Our Timings" << std::endl;

    // Generate pp and msk.
    auto pp = Filter::pp_gen(1, 20);
    auto msk = Filter::msk_gen(pp);

    for (int num_col = 1; num_col <= 20; ++num_col){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            auto y = Helper::rand_int_vec(num_col, 1, std::numeric_limits<int>::max());

            // Set selection.
            IntVec sel;
            for (int j = 0; j < num_col; ++j){ sel.push_back(j); }

            // Total timings.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = Filter::keygen(pp, msk, y, sel);
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Compute ciphertext.
            auto ct = Filter::enc(pp, msk, x);

            // Decryption time.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = Filter::dec(pp, ct, sk, sel);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;
    }

    // Close the BP.
    BP::close();

    // Create some blank spaces.
    file << std::endl << std::endl;
}

void bench_total_single_filter_time(const int round){
    ipe_total_single_filter_time(round);
    sse_total_single_filter_time(round);
    our_total_single_filter_time(round);
}
