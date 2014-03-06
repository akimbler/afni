static char * PvalueStuff[] = {
   "\n" ,
   "---------------------\n" ,
   "A NOTE ABOUT p-VALUES\n" ,
   "---------------------\n" ,
   "The 2-sided p-value of a t-statistic value T is the likelihood (probability)\n" ,
   "that the absolute value of the t-statistic computation would be bigger than\n" ,
   "the absolute value of T, IF the null hypothesis of no difference in the means\n" ,
   "(2-sample test) were true.  For example, with 30 degrees of freedom, a T-value\n" ,
   "of 2.1 has a p-value of 0.0442 -- that is, if the null hypothesis is true\n" ,
   "and you repeated the experiment a lot of times, only 4.42% of the time would\n" ,
   "the T-value get to be 2.1 or bigger (and -2.1 or more negative).\n" ,
   "\n" ,
   "You can NOT interpret this to mean that the alternative hypothesis (that the\n" ,
   "means are different) is 95.58% likely to be true.  (After all, this T-value\n" ,
   "shows a pretty weak effect size -- difference in the means for a 2-sample\n" ,
   "t-test, magnitude of the mean for a 1-sample t-test, scaled by the standard\n" ,
   "deviation of the noise in the samples.)  A better way to think about it is\n" ,
   "to pose the following question:\n" ,
   "     Assuming that the alternative hypothesis is true, how likely\n" ,
   "     is it that you would get the p-value of 0.0442, versus how\n" ,
   "     likely is p=0.0442 when the null hypothesis is true?\n" ,
   "This is the question addressed in the paper\n" ,
   "     Calibration of p Values for Testing Precise Null Hypotheses.\n" ,
   "     T Sellke, MJ Bayarri, and JO Berger.\n" ,
   "     The American Statistician v.55:62-71, 2001.\n" ,
   "     http://www.stat.duke.edu/courses/Spring10/sta122/Labs/Lab6.pdf\n" ,
   "The exact interpretation of what the above question means is somewhat\n" ,
   "tricky, depending on if you are a Bayesian heretic or a Frequentist\n" ,
   "true believer.  But in either case, one reasonable answer is given by\n" ,
   "the function\n" ,
   "     alpha(p) = 1 / [ 1 - 1/( e * p * log(p) ) ]\n" ,
   "(where 'e' is 2.71828... and 'log' is to the base 'e').  Here,\n" ,
   "alpha(p) can be interpreted as the likelihood that the given p-value\n" ,
   "was generated by the null hypothesis, versus being from the alternative\n" ,
   "hypothesis.  For p=0.0442, alpha=0.2726; in non-quantitative words, this\n" ,
   "p-value is NOT very strong evidence that the alternative hypothesis is true.\n" ,
   "\n" ,
   "Why is this so -- why isn't saying 'the null hypothesis would only give\n" ,
   "a result this big 4.42% of the time' similar to saying 'the alternative\n" ,
   "hypothesis is 95.58% likely to be true'?  The answer is because it is\n" ,
   "only somewhat more likely the t-statistic would be that value when the\n" ,
   "alternative hypothesis is true.  In this example, the difference in means\n" ,
   "cannot be very large, or the t-statistic would almost certainly be larger.\n" ,
   "But with a small difference in means (relative to the standard deviation),\n" ,
   "the alternative hypothesis (noncentral) t-value distribution isn't that\n" ,
   "different that the null hypothesis (central) t-value distribution.  It is\n" ,
   "true that the alternative hypothesis is more likely to be true than the\n" ,
   "null hypothesis (when p < 1/e = 0.36788), but it isn't AS much more likely\n" ,
   "to be true than the p-value itself seems to say.\n" ,
   "\n" ,
   "Some values of alpha(p) for those too lazy to calculate just now:\n" ,
   "     p = 0.0005 alpha = 0.010225\n" ,
   "     p = 0.001  alpha = 0.018431\n" ,
   "     p = 0.005  alpha = 0.067174\n" ,
   "     p = 0.010  alpha = 0.111254\n" ,
   "     p = 0.015  alpha = 0.146204\n" ,
   "     p = 0.020  alpha = 0.175380\n" ,
   "     p = 0.030  alpha = 0.222367\n" ,
   "     p = 0.040  alpha = 0.259255\n" ,
   "     p = 0.050  alpha = 0.289350\n" ,
   "You can also try this AFNI package command to plot alpha(p) vs. p:\n" ,
   "     1deval -dx 0.001 -xzero 0.001 -num 99 -expr '1/(1-1/(exp(1)*p*log(p)))' |\n" ,
   "       1dplot -stdin -dx 0.001 -xzero 0.001 -xlabel 'p' -ylabel '\\alpha(p)'\n" ,
   "Another example: to reduce the likelihood of the null hypothesis being the\n" ,
   "source of your t-statistic to 10%, you have to have p = 0.008593 -- a value\n" ,
   "more stringent than usually seen in scientific publications!\n" ,
   "\n" ,
   "Finally, none of the discussion above is limited to the case of p-values that\n" ,
   "come from t-tests.  The function alpha(p) applies (approximately) to many\n" ,
   "other situations.  See the paper by Sellke et al. for a lengthier and more\n" ,
   "precise discussion.  Another paper on the same topic is the more recent\n" ,
   "     Revised standards for statistical evidence.\n" ,
   "     VE Johnson.  PNAS v110:19313-19317, 2013.\n" ,
   "     http://www.pnas.org/content/110/48/19313.long\n" ,
   "What I have tried to do herein is outline the p-value issue in non-technical\n" ,
   "and approximate terms.\n" ,
   NULL } ;
#define NUM_PvalueStuff 76
