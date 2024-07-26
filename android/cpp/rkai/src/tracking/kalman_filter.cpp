#include "kalman_filter.h"


KalmanFilter::KalmanFilter(unsigned int num_states, unsigned int num_obs) :
        num_states_(num_states), num_obs_(num_obs) {
    /*** Predict ***/
    // State vector
    // x_ = Eigen::VectorXd::Zero(num_states);
    x_.resize(num_states, 1, 0); 
    // Predicted(a prior) state vector
    // x_predict_ = Eigen::VectorXd::Zero(num_states);
    x_predict_.resize(num_states, 1, 0);

    // State transition matrix F_
    // F_ = Eigen::MatrixXd::Zero(num_states, num_states);
    F_.resize(num_states, num_states, 0);

    // Error covariance matrix P
    // P_ = Eigen::MatrixXd::Zero(num_states, num_states);
    P_.resize(num_states, num_states, 0);

    // Predicted(a prior) error covariance matrix
    // P_predict_ = Eigen::MatrixXd::Zero(num_states, num_states);
    P_predict_.resize(num_states, num_states, 0);

    // Covariance matrix of process noise
    // Q_ = Eigen::MatrixXd::Zero(num_states, num_states);
    Q_.resize(num_states, num_states, 0);

    /*** Update ***/
    // Observation matrix
    // H_ = Eigen::MatrixXd::Zero(num_obs, num_states);
    H_.resize(num_obs, num_states, 0);

    // Covariance matrix of observation noise
    // R_ = Eigen::MatrixXd::Zero(num_obs, num_obs);
    R_.resize(num_obs, num_obs, 0);

    log_likelihood_delta_ = 0.0;
    NIS_ = 0.0;
}


void KalmanFilter::Coast() {
    x_predict_ = F_ * x_;

    //Tranpose F_
    DDMatrix F_t_;
    F_.transpose(F_t_);
    
    P_predict_ = F_ * P_ * F_t_ + Q_;
}


void KalmanFilter::Predict() {
    Coast();
    x_ = x_predict_;
    P_ = P_predict_;
}


DDMatrix KalmanFilter::PredictionToObservation(DDMatrix state) {
    return (H_*state);
}


void KalmanFilter::Update(DDMatrix z) {
    DDMatrix z_predict = PredictionToObservation(x_predict_);
    
    // y - innovation, z - real observation, z_predict - predicted observation
    DDMatrix y = z - z_predict;
    DDMatrix Ht; 
    H_.transpose(Ht);

    // S - innovation covariance
    DDMatrix S = H_ * P_predict_ * Ht + R_;

    DDMatrix y_t;
    y.transpose(y_t);
    DDMatrix M_NIS = y_t * (! S) * y; 
    NIS_ = M_NIS(0, 0);

//    std::cout << std::endl;
//    std::cout << "P_predict = " << std::endl;
//    std::cout << P_predict_ << std::endl;
//
//
//    std::cout << "Z = " << std::endl;
//    std::cout << z << std::endl;
//
//    std::cout << "Z_pred = " << std::endl;
//    std::cout << z_predict << std::endl;
//
//    std::cout << "y = " << std::endl;
//    std::cout << y << std::endl;
//
//    std::cout << "S = " << std::endl;
//    std::cout << S << std::endl;
//
//    std::cout << "NIS = " << NIS_ << std::endl;


    // K - Kalman gain
    DDMatrix K = P_predict_ * Ht * (! S);

    // Updated state estimation
    x_ = x_predict_ + K * y;

    // DDMatrix I = Eigen::MatrixXd::Identity(num_states_, num_states_);
    DDMatrix i_matrix (num_states_, num_states_, 0.);
    i_matrix.identity();
    // Joseph form
    //P_ = (I - K * H_) * P_predict_ * (I - K * H_).transpose() + K * R_ * K.transpose();
    // Optimal gain
    P_ = (i_matrix - K * H_) * P_predict_;
}


// float KalmanFilter::CalculateLogLikelihood(DDMatrix y, DDMatrix S) {
//     float log_likelihood;

//     // Note: Computing log(M.determinant()) in Eigen C++ is risky for large matrices since it may overflow or underflow.
//     // compute the Cholesky decomposition of the innovation covariance matrix, because it is symmetric
//     // S = L * L^T = U^T * U
//     // then retrieve factor L in the decomposition
//     auto& L = S.llt().matrixL();

//     // find log determinant of innovation covariance matrix
//     float log_determinant = 0;
//     for (unsigned int i = 0; i < S.rows(); i++)
//         log_determinant += log(L(i, i));
//     log_determinant *= 2;

//     // log-likelihood expression for current iteration
//     log_likelihood = -0.5 * (y.transpose() * S.inverse() * y + num_obs_ * log(2 * M_PI) + log_determinant);

//     if (std::isnan(log_likelihood)) {
//     	log_likelihood = -1e50;
// 	}

// 	return log_likelihood;
// }