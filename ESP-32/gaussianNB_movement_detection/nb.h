#ifndef NB_H_INCLUDED
#define NB_H_INCLUDED
namespace Eloquent {
  namespace ML {
    namespace Port {
      class GaussianNB {
        public:
          /**
            Predict class for features vector
          */
          int predict(float *x) {
            float votes[2] = { 0.0f };
            float theta[1] = { 0 };
            float sigma[1] = { 0 };
            theta[0] = 7.43541;
            sigma[0] = 4.215593433987;
            votes[0] = 0.5 - gauss(x, theta, sigma);
            theta[0] = 303.243472;
            sigma[0] = 114968.75330482732;
            votes[1] = 0.5 - gauss(x, theta, sigma);
            // return argmax of votes
            uint8_t classIdx = 0;
            float maxVotes = votes[0];
      
            for (uint8_t i = 1; i < 2; i++) {
              if (votes[i] > maxVotes) {
                classIdx = i;
                maxVotes = votes[i];
              }
            }
      
            return classIdx;
          }
      
        protected:
          /**
            Compute gaussian value
          */
          float gauss(float *x, float *theta, float *sigma) {
            float gauss = 0.0f;
      
            for (uint16_t i = 0; i < 1; i++) {
              gauss += log(sigma[i]);
              gauss += abs(x[i] - theta[i]) / sigma[i];
            }
      
            return gauss;
          }
      };
    }
  }
}
#endif
