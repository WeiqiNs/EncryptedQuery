#include "exp.hpp"
#include "kim_aggre.hpp"

void ipe_equality_time(const int round){
    // Open the output files.
    std::ofstream file("equality_time.txt", std::ios_base::app);
    file << "IPE Equality Timings" << std::endl;

    for (int num_col : {2, 10, 20}){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Create pp and msk.
        auto pp = IpeAggre::pp_gen(20);
        auto msk = IpeAggre::msk_gen(pp);

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            auto y = Helper::rand_int_vec(20, 1, 5);
            // Set the unselected portion to zero.
            for (int j = num_col + 1; j < 20; ++j){ y[j] = 0; }

            // Compute ct and sk.
            auto ct = IpeAggre::enc(pp, msk, x);

            // Keygen timing.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = IpeAggre::keygen(pp, msk, y, 100);
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Decryption timings.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = IpeAggre::dec(ct, sk);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;

        // Close the BP.
        BP::close();
    }
    // Create some blank spaces.
    file << std::endl << std::endl;
}

void our_equality_time(const int round){
    // Open the output files.
    std::ofstream file("equality_time.txt", std::ios_base::app);
    file << "Our Equality Timings" << std::endl;

    for (int num_col : {2, 10, 20}){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Create pp and msk.
        auto pp = Aggre::pp_gen(20);
        auto msk = Aggre::msk_gen(pp);

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            // Create a random vector of desired length.
            auto y = Helper::rand_int_vec(num_col, 1, 5);

            // Set the unselected portion to zero.
            IntVec sel;
            for (int j = 0; j < num_col; ++j){ sel.push_back(j); }

            // Compute ct and sk.
            auto ct = Aggre::enc(pp, msk, x);

            // Keygen timing.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = Aggre::keygen(pp, msk, y, 100, sel);
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Decryption timings.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = Aggre::dec(ct, sk, sel);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;

        // Close the BP.
        BP::close();
    }
    // Create some blank spaces.
    file << std::endl << std::endl;
}

void kim_equality_time(const int round){
    // Open the output files.
    std::ofstream file("equality_time.txt", std::ios_base::app);
    file << "Kim Equality Timings" << std::endl;

    for (int num_col : {2, 10, 20}){
        // Create holder for timings.
        std::chrono::duration<double, std::milli> time{};

        // Create pp and msk.
        auto pp = KimAggre::pp_gen(20);
        auto msk = KimAggre::msk_gen(pp);

        // Perform round number of Enc.
        for (int i = 0; i < round; ++i){
            // Create a random vector of desired length.
            auto x = Helper::rand_int_vec(20, 1, std::numeric_limits<int>::max());
            auto y = Helper::rand_int_vec(20, 1, 5);
            // Set the unselected portion to zero.
            for (int j = num_col + 1; j < 20; ++j){ y[j] = 0; }

            // Compute ct and sk.
            auto ct = KimAggre::enc(pp, msk, x);

            // Keygen timing.
            auto start = std::chrono::high_resolution_clock::now();
            auto sk = KimAggre::keygen(pp, msk, y, 100);
            auto end = std::chrono::high_resolution_clock::now();
            time += end - start;

            // Decryption timings.
            start = std::chrono::high_resolution_clock::now();
            std::ignore = KimAggre::dec(ct, sk);
            end = std::chrono::high_resolution_clock::now();
            time += end - start;
        }

        // Output the time.
        file << "(" << num_col << ", " << time.count() / round << ")" << std::endl;

        // Close the BP.
        BP::close();
    }
    // Create some blank spaces.
    file << std::endl << std::endl;
}

void bench_equality_time(const int round){
    ipe_equality_time(round);
    our_equality_time(round);
    kim_equality_time(round);
}
