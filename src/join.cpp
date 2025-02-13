#include "join.hpp"

JoinPP Join::pp_gen(const int degree, const int length, const bool pre){
    // Create the pp instance.
    JoinPP pp;
    // Update the degree according to input.
    pp.d = degree;
    // Update the input length according to input.
    pp.l = length;
    // Save the created pairing group.
    pp.pairing_group = std::make_unique<BP>(pre);

    return pp;
}

JoinMsk Join::msk_gen(const JoinPP& pp, const CharVec& key){
    // Create the msk instance.
    JoinMsk msk;
    // Get the unique point for HMAC.
    msk.hmac = std::make_unique<HMAC>(key);
    // Sample the random point k and l.
    msk.k = pp.pairing_group->Zp->rand();

    // These values are only needed for d > 1.
    if (pp.d > 1){
        // Sample a random point and find its inverse.
        msk.d = pp.pairing_group->Zp->rand();
        msk.di = pp.pairing_group->Zp->inv(msk.d);
        // If degree is higher than 1, we need the length to be 2 * ((length - 1) * degree + 1).
        // This utilizes the optimization of adding the constant together.
        msk.r = pp.pairing_group->Zp->rand_vec(2 * ((pp.l - 1) * pp.d + 1));
        msk.b = pp.pairing_group->Zp->rand_vec(2 * ((pp.l - 1) * pp.d + 1));
        msk.bi = pp.pairing_group->Zp->vec_inv(msk.b);
    }

    // Return the master secret key.
    return msk;
}

G1Vec Join::enc(const JoinPP& pp, const JoinMsk& msk, const Vec& x, const int join_index){
    // Declare variable to hold the point to join on.
    Fp join_on;

    // Make a copy of the input x.
    Vec x_copy = x;

    // Compute the hash of value to join and remove the join value from x copy.
    std::visit([&pp, &msk, &join_on, join_index](auto&& input_x){
        using T = std::decay_t<decltype(input_x)>;
        if constexpr (std::is_same_v<T, IntVec>){
            join_on = Helper::char_vec_to_fp(msk.hmac->digest(Helper::int_to_char_vec(input_x[join_index])));
            pp.pairing_group->Zp->mod(join_on);
            input_x.erase(input_x.begin() + join_index);
        }
        else if constexpr (std::is_same_v<T, StrVec>){
            join_on = Helper::char_vec_to_fp(msk.hmac->digest(Helper::str_to_char_vec(input_x[join_index])));
            pp.pairing_group->Zp->mod(join_on);
            input_x.erase(input_x.begin() + join_index);
        }
        else throw std::invalid_argument("The degree must be 1 when inputting a y vector.");
    }, x_copy);

    // Generate hash of the input x vector.
    const auto x_digest = msk.hmac->digest_vec_to_fp_mod(*pp.pairing_group, x_copy);

    // Sample the random point alpha.
    const auto alpha = pp.pairing_group->Zp->rand();

    // Depends on whether the degree is 1 or higher, we perform encryption differently.
    if (pp.d == 1){
        // Find a * H(x).
        auto ax = pp.pairing_group->Zp->vec_mul(x_digest, alpha);
        // Append join + ak.
        ax.push_back(pp.pairing_group->Zp->add(join_on, pp.pairing_group->Zp->mul(msk.k, alpha)));
        // Append the last point negative a.
        ax.push_back(pp.pairing_group->Zp->neg(alpha));
        // Raise the vector to g1 and return.
        return pp.pairing_group->Gp->g1_raise(ax);
    }

    // Here is the case where the degree is higher than 1, we compute the value for evaluating polynomial at x.
    const auto poly_x = Helper::power_poly(pp.d, *pp.pairing_group, x_digest);
    // Duplicate the poly(x).
    const auto xx = Field::vec_join(poly_x, poly_x);
    // Compute poly(x)||poly(x) + r.
    const auto xxr = pp.pairing_group->Zp->vec_add(xx, msk.r);
    // Compute alpha * (poly(x)||poly(x) + r).
    const auto axxr = pp.pairing_group->Zp->vec_mul(xxr, alpha);
    // Compute alpha * vec_b * (poly(x)||poly(x) + r).
    auto abxxr = pp.pairing_group->Zp->vec_mul(axxr, msk.b);
    // Also attach join + k * alpha.
    abxxr.push_back(pp.pairing_group->Zp->add(join_on, pp.pairing_group->Zp->mul(msk.k, alpha)));
    // Attach the last point -alpha * delta.
    abxxr.push_back(pp.pairing_group->Zp->neg(pp.pairing_group->Zp->mul(alpha, msk.d)));

    // Raise the vector to g1 and return.
    return pp.pairing_group->Gp->g1_raise(abxxr);
}

G2Vec Join::keygen(const JoinPP& pp, const JoinMsk& msk, const VecOrMat& y, const IntVec& sel){
    // Sample the random point beta.
    const auto beta = pp.pairing_group->Zp->rand();

    // Depends on whether the degree is 1 or higher, we perform key generation differently.
    if (pp.d == 1){
        // Generate the hash of the input y vector.
        Fp digest_sum;

        // Make sure the input y is vector type when degree is 1.
        std::visit([&pp, &msk, &sel, &digest_sum](auto&& input_y){
            using T = std::decay_t<decltype(input_y)>;
            if constexpr (std::is_same_v<T, IntVec> || std::is_same_v<T, StrVec>){
                FpVec digest = msk.hmac->digest_vec_to_fp_mod(*pp.pairing_group, input_y, sel);
                digest.push_back(msk.k);
                digest_sum = pp.pairing_group->Zp->vec_sum(digest);
            }
            else throw std::invalid_argument("The degree must be 1 when inputting a y vector.");
        }, y);

        // Raise the vector to g2 and return.
        return pp.pairing_group->Gp->g2_raise(FpVec{Fp(1), digest_sum});
    }

    // Generate the hash of the input y vector.
    FpMat y_digest;
    // Make sure the input y is vector type.
    std::visit([&pp, &msk, &sel, &y_digest](auto&& input_y){
        using T = std::decay_t<decltype(input_y)>;
        if constexpr (std::is_same_v<T, IntMat> || std::is_same_v<T, StrMat>)
            y_digest = msk.hmac->digest_mat_to_fp_mod(*pp.pairing_group, input_y, sel);
        else throw std::invalid_argument("The degree must be more than 1 when inputting a y matrix.");
    }, y);

    // We compute the coefficient for when mat y equals to zero.
    auto coeff = Helper::coeff_poly(pp.d, *pp.pairing_group, y_digest);
    // Split the coefficient to two parts.
    coeff = Helper::split_poly(*pp.pairing_group, coeff);

    // Select r and bi.
    FpVec sel_r, sel_bi;

    // Depends on whether sel is needed, we build the vectors.
    if (sel.empty()){
        sel_r = msk.r;
        sel_bi = msk.bi;
    }
    // When sel is provided, filter out the keys.
    else{
        // Get the selected index.
        const auto sel_index = Helper::get_sel_index(pp.d, pp.l - 1, sel);

        for (const auto i : sel_index){
            sel_r.push_back(msk.r[i]);
            sel_bi.push_back(msk.bi[i]);
        }
        for (const auto i : sel_index){
            sel_r.push_back(msk.r[msk.r.size() / 2 + i]);
            sel_bi.push_back(msk.bi[msk.r.size() / 2 + i]);
        }
    }

    // Compute beta * c.
    const auto bc = pp.pairing_group->Zp->vec_mul(coeff, beta);
    // Compute beta * bi * c.
    auto bbic = pp.pairing_group->Zp->vec_mul(bc, sel_bi);

    // Compute the last point to join to the vector.
    bbic.push_back(Fp{1});
    auto temp = pp.pairing_group->Zp->vec_ip(coeff, sel_r);
    temp = pp.pairing_group->Zp->mul(temp, beta);
    temp = pp.pairing_group->Zp->add(temp, msk.k);
    bbic.push_back(pp.pairing_group->Zp->mul(temp, msk.di));

    // Raise to g2 and return.
    return pp.pairing_group->Gp->g2_raise(bbic);
}

Gt Join::dec(const JoinPP& pp, const G1Vec& ct, const G2Vec& sk, const IntVec& sel){
    if (pp.d == 1){
        // We select desired things from ct.
        G1Vec sel_ct;
        if (sel.empty()) sel_ct = G1Vec(ct.begin(), ct.end() - 2);
        else for (const auto i : sel) sel_ct.push_back(ct[i]);

        // We add the second to last point as well.
        sel_ct.push_back(ct[ct.size() - 2]);

        // Sum the selected ct and add the last point.
        const G1Vec pair_ct{pp.pairing_group->Gp->g1_add_vec(sel_ct), ct.back()};

        /// Compute the pairing and output filter result.
        return Group::pair(pair_ct, sk);
    }
    // If sel is empty.
    if (sel.empty()) return Group::pair(ct, sk);

    // Get the selected index.
    const auto sel_index = Helper::get_sel_index(pp.d, pp.l - 1, sel);

    // Create the holder for selected ct.
    G1Vec sel_ct;
    for (const auto i : sel_index) sel_ct.push_back(ct[i]);
    for (const auto i : sel_index) sel_ct.push_back(ct[ct.size() / 2 + i - 1]);
    // We also need to add the last two points in ct.
    sel_ct.push_back(ct[ct.size() - 2]);
    sel_ct.push_back(ct[ct.size() - 1]);

    return Group::pair(sel_ct, sk);
}
