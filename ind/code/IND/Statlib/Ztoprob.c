/*
 *      returns x st. Q(x)=p,
 *      i.e. x is a Z-score, Q(x) is error in normal distribution 
 *      26.2.22 in Abramowitz & Stegun
 */
float Ztoprob(p)
float p;
{
        double sqrt(), log();
        float t = sqrt(-2.0 * log((double)p));
        return (float) t - (2.30753+0.27061*t)/(1.0+0.99229*t+0.04481*t*t);
}
