//Polyohase FIR filter coefficents for Oversampling

const float polyphaseFIRcoefs_4[96] = {0,-0.000089420931270978,-0.0002010766891,-0.0002111746303,0,0.0004102492819,0.0007740754133,0.0007147569137,0,-0.001158372906,-0.002042805529,-0.001780704474,0,0.002627694031,0.004457108117,0.003752153484,0,-0.005214584823,-0.008618027479,-0.007084531631,0,0.009442846969,
0.01532214346,0.01238549626,0,-0.01602992044,-0.0256826399,-0.02052573092,0,0.02607295427,0.04147118748,0.03295401713,0,-0.04159220087,-0.06613834255,-0.05266489232,0,0.06734108688,0.1084038132,0.08783600994,0,-0.1189165043,-0.2003420047,-0.1730184801,0,0.2958313541,0.6325760967,
0.898883675,1,0.898883675,0.6325760967,0.2958313541,0,-0.1730184801,-0.2003420047,-0.1189165043,0,0.08783600994,0.1084038132,0.067341086880,-0.05266489232,-0.06613834255,
-0.04159220087,0,0.03295401713,0.04147118748,0.02607295427,0,-0.02052573092,-0.0256826399,-0.01602992044,0,0.01238549626,0.01532214346,0.009442846969,0,-0.007084531631,-0.008618027479,-0.005214584823,0,0.003752153484,0.004457108117,0.002627694031,0,-0.001780704474,-0.002042805529,-0.001158372906,
0,0.0007147569137,0.0007740754133,0.0004102492819,0,-0.0002111746303,-0.0002010766891,-0.000089420931270978
};

const float polyphaseFIRcoefs_4_reversed[96] = {-0.000089420931270978,-0.0002010766891,-0.0002111746303,0,0.0004102492819,0.0007740754133,0.0007147569137,0,-0.001158372906,-0.002042805529,-0.001780704474,0,0.002627694031,0.004457108117,0.003752153484,0,-0.005214584823,-0.008618027479,-0.007084531631,0,0.009442846969,
0.01532214346,0.01238549626,0,-0.01602992044,-0.0256826399,-0.02052573092,0,0.02607295427,0.04147118748,0.03295401713,0,-0.04159220087,-0.06613834255,-0.05266489232,0,0.06734108688,0.1084038132,0.08783600994,0,-0.1189165043,-0.2003420047,-0.1730184801,0,0.2958313541,0.6325760967,
0.898883675,1,0.898883675,0.6325760967,0.2958313541,0,-0.1730184801,-0.2003420047,-0.1189165043,0,0.08783600994,0.1084038132,0.067341086880,-0.05266489232,-0.06613834255,
-0.04159220087,0,0.03295401713,0.04147118748,0.02607295427,0,-0.02052573092,-0.0256826399,-0.01602992044,0,0.01238549626,0.01532214346,0.009442846969,0,-0.007084531631,-0.008618027479,-0.005214584823,0,0.003752153484,0.004457108117,0.002627694031,0,-0.001780704474,-0.002042805529,-0.001158372906,
0,0.0007147569137,0.0007740754133,0.0004102492819,0,-0.0002111746303,-0.0002010766891,-0.000089420931270978,0
};


const float polyphaseFIRcoefs_6[144] = {0,-0.0000530072485187154,-0.0001290614862,-0.0002010766891,-0.0002280403163,-0.0001684973702,0,0.000262035748,0.0005546137386,0.0007740754133,0.0008026074303,0.0005501562179,0,-0.0007588470574,-0.001529123503,-0.002042805529,-0.002036387807,-0.001346989195,0,0.001745688527,0.00342230706,
0.004457108117,0.00433928347,0.002807684058,0,-0.003496324479,-0.006731093724,-0.008618027479,-0.008256378144,-0.005261787503,0,0.006372467967,0.01211254026,0.01532214346,0.0145130044,0.00915038426,0,-0.01086677004,-0.02047215835,-0.0256826399,-0.02413925292,-0.01511148857,0,0.01772345683,0.03321251426,
0.04147118748,0.03882308985,0.02422362826,0,-0.0282885659,-0.05296400188,-0.06613834255,-0.06198370246,-0.03876237002,0,0.045658078,0.08606104175,0.1084038132,0.1027127741,0.06511574397,0,-0.07964984885,-0.1540485155,-0.2003420047,-0.197567026,-0.1317587985,0,0.187632816,0.4088375251,0.6325760967,
0.8246551151,0.954254044,1,0.954254044,0.8246551151,0.6325760967,0.4088375251,0.187632816,0,-0.1317587985,-0.197567026,-0.2003420047,-0.1540485155,-0.07964984885,0,0.06511574397,0.1027127741,0.1084038132,0.08606104175,0.045658078,0,-0.03876237002,-0.06198370246,-0.06613834255,-0.05296400188,-0.0282885659,
0,0.02422362826,0.03882308985,0.04147118748,0.03321251426,0.01772345683,0,-0.01511148857,-0.02413925292,-0.0256826399,-0.02047215835,-0.01086677004,0,0.00915038426,0.0145130044,0.01532214346,0.01211254026,0.006372467967,0,-0.005261787503,-0.008256378144,-0.008618027479,-0.006731093724,-0.003496324479,0,
0.002807684058,0.00433928347,0.004457108117,0.00342230706,0.001745688527,0,-0.001346989195,-0.002036387807,-0.002042805529,-0.001529123503,-0.0007588470574,0,0.0005501562179,0.0008026074303,0.0007740754133,0.0005546137386,0.000262035748,0,-0.0001684973702,-0.0002280403163,-0.0002010766891,-0.0001290614862,-0.0000530072485187154
};

const float polyphaseFIRcoefs_6_reversed[144] = {-0.0000530072485187154,-0.0001290614862,-0.0002010766891,-0.0002280403163,-0.0001684973702,0,0.000262035748,0.0005546137386,0.0007740754133,0.0008026074303,0.0005501562179,0,-0.0007588470574,-0.001529123503,-0.002042805529,-0.002036387807,-0.001346989195,0,0.001745688527,0.00342230706,
0.004457108117,0.00433928347,0.002807684058,0,-0.003496324479,-0.006731093724,-0.008618027479,-0.008256378144,-0.005261787503,0,0.006372467967,0.01211254026,0.01532214346,0.0145130044,0.00915038426,0,-0.01086677004,-0.02047215835,-0.0256826399,-0.02413925292,-0.01511148857,0,0.01772345683,0.03321251426,
0.04147118748,0.03882308985,0.02422362826,0,-0.0282885659,-0.05296400188,-0.06613834255,-0.06198370246,-0.03876237002,0,0.045658078,0.08606104175,0.1084038132,0.1027127741,0.06511574397,0,-0.07964984885,-0.1540485155,-0.2003420047,-0.197567026,-0.1317587985,0,0.187632816,0.4088375251,0.6325760967,
0.8246551151,0.954254044,1,0.954254044,0.8246551151,0.6325760967,0.4088375251,0.187632816,0,-0.1317587985,-0.197567026,-0.2003420047,-0.1540485155,-0.07964984885,0,0.06511574397,0.1027127741,0.1084038132,0.08606104175,0.045658078,0,-0.03876237002,-0.06198370246,-0.06613834255,-0.05296400188,-0.0282885659,
0,0.02422362826,0.03882308985,0.04147118748,0.03321251426,0.01772345683,0,-0.01511148857,-0.02413925292,-0.0256826399,-0.02047215835,-0.01086677004,0,0.00915038426,0.0145130044,0.01532214346,0.01211254026,0.006372467967,0,-0.005261787503,-0.008256378144,-0.008618027479,-0.006731093724,-0.003496324479,0,
0.002807684058,0.00433928347,0.004457108117,0.00342230706,0.001745688527,0,-0.001346989195,-0.002036387807,-0.002042805529,-0.001529123503,-0.0007588470574,0,0.0005501562179,0.0008026074303,0.0007740754133,0.0005546137386,0.000262035748,0,-0.0001684973702,-0.0002280403163,-0.0002010766891,-0.0001290614862,-0.0000530072485187154,0
};


const float polyphaseFIRcoefs_8[192] = {0,-0.0000369464911936416,-0.000089420931270978,-0.000148882116,-0.0002010766891,-0.0002279542535,-0.0002111746303,-0.0001367537005,0,0.0001903855486,0.0004102492819,0.0006209758261,0.0007740754133,0.000819218344,0.0007147569137,0.000439005601,0,-0.0005587126577,-0.001158372906,-0.001692676233,-0.002042805529,
-0.00209829715,-0.001780704474,-0.00106585844,0,0.001294606301,0.002627694031,0.003763648926,0.004457108117,0.00449700521,0.003752153484,0.002209989347,0,-0.002605134359,-0.005214584823,-0.007370222985,-0.008618027479,-0.00859018216,-0.007084531631,-0.004126551687,0,0.004763901376,0.009442846969,0.01322185622,
0.01532214346,0.01514185984,0.01238549626,0.007157655667,0,-0.008142580703,-0.01602992044,-0.02229948546,-0.0256826399,-0.02523249929,-0.02052573092,-0.01180060211,0,0.01329933236,0.02607295427,0.03613254865,0.04147118748,0.04061941176,0.03295401713,0.01890286033,0,-0.02123480629,-0.04159220087,-0.05761619059,
-0.06613834255,-0.06482699166,-0.05266489232,-0.03027123233,0,0.03422500228,0.06734108688,0.0938052846,0.1084038132,0.107105515,0.08783600994,0.05105019953,0,-0.05936983683,-0.1189165043,-0.169148677,-0.2003420047,-0.2037882475,-0.1730184801,-0.1048345871,0,0.1365210719,0.2958313541,0.465865445,0.6325760967,
0.7814079897,0.898883675,0.9741074862,1,0.9741074862,0.898883675,0.7814079897,0.6325760967,0.465865445,0.2958313541,0.1365210719,0,-0.1048345871,-0.1730184801,-0.2037882475,-0.2003420047,-0.169148677,-0.1189165043,-0.05936983683,0,0.05105019953,0.08783600994,0.107105515,0.1084038132,0.0938052846,0.06734108688,
0.03422500228,0,-0.03027123233,-0.05266489232,-0.06482699166,-0.06613834255,-0.05761619059,-0.04159220087,-0.02123480629,0,0.01890286033,0.03295401713,0.04061941176,0.04147118748,0.03613254865,0.02607295427,0.01329933236,0,-0.01180060211,-0.02052573092,-0.02523249929,-0.0256826399,-0.02229948546,-0.01602992044,
-0.008142580703,0,0.007157655667,0.01238549626,0.01514185984,0.01532214346,0.01322185622,0.009442846969,0.004763901376,0,-0.004126551687,-0.007084531631,-0.00859018216,-0.008618027479,-0.007370222985,-0.005214584823,-0.002605134359,0,0.002209989347,0.003752153484,0.00449700521,0.004457108117,0.003763648926,
0.002627694031,0.001294606301,0,-0.00106585844,-0.001780704474,-0.00209829715,-0.002042805529,-0.001692676233,-0.001158372906,-0.0005587126577,0,0.000439005601,0.0007147569137,0.000819218344,0.0007740754133,0.0006209758261,0.0004102492819,0.0001903855486,0,-0.0001367537005,-0.0002111746303,-0.0002279542535,
-0.0002010766891,-0.000148882116,-0.000089420931270978,-0.0000369464911936416
};

const float polyphaseFIRcoefs_8_reversed[192] = {-0.0000369464911936416,-0.000089420931270978,-0.000148882116,-0.0002010766891,-0.0002279542535,-0.0002111746303,-0.0001367537005,0,0.0001903855486,0.0004102492819,0.0006209758261,0.0007740754133,0.000819218344,0.0007147569137,0.000439005601,0,-0.0005587126577,-0.001158372906,-0.001692676233,-0.002042805529,
-0.00209829715,-0.001780704474,-0.00106585844,0,0.001294606301,0.002627694031,0.003763648926,0.004457108117,0.00449700521,0.003752153484,0.002209989347,0,-0.002605134359,-0.005214584823,-0.007370222985,-0.008618027479,-0.00859018216,-0.007084531631,-0.004126551687,0,0.004763901376,0.009442846969,0.01322185622,
0.01532214346,0.01514185984,0.01238549626,0.007157655667,0,-0.008142580703,-0.01602992044,-0.02229948546,-0.0256826399,-0.02523249929,-0.02052573092,-0.01180060211,0,0.01329933236,0.02607295427,0.03613254865,0.04147118748,0.04061941176,0.03295401713,0.01890286033,0,-0.02123480629,-0.04159220087,-0.05761619059,
-0.06613834255,-0.06482699166,-0.05266489232,-0.03027123233,0,0.03422500228,0.06734108688,0.0938052846,0.1084038132,0.107105515,0.08783600994,0.05105019953,0,-0.05936983683,-0.1189165043,-0.169148677,-0.2003420047,-0.2037882475,-0.1730184801,-0.1048345871,0,0.1365210719,0.2958313541,0.465865445,0.6325760967,
0.7814079897,0.898883675,0.9741074862,1,0.9741074862,0.898883675,0.7814079897,0.6325760967,0.465865445,0.2958313541,0.1365210719,0,-0.1048345871,-0.1730184801,-0.2037882475,-0.2003420047,-0.169148677,-0.1189165043,-0.05936983683,0,0.05105019953,0.08783600994,0.107105515,0.1084038132,0.0938052846,0.06734108688,
0.03422500228,0,-0.03027123233,-0.05266489232,-0.06482699166,-0.06613834255,-0.05761619059,-0.04159220087,-0.02123480629,0,0.01890286033,0.03295401713,0.04061941176,0.04147118748,0.03613254865,0.02607295427,0.01329933236,0,-0.01180060211,-0.02052573092,-0.02523249929,-0.0256826399,-0.02229948546,-0.01602992044,
-0.008142580703,0,0.007157655667,0.01238549626,0.01514185984,0.01532214346,0.01322185622,0.009442846969,0.004763901376,0,-0.004126551687,-0.007084531631,-0.00859018216,-0.008618027479,-0.007370222985,-0.005214584823,-0.002605134359,0,0.002209989347,0.003752153484,0.00449700521,0.004457108117,0.003763648926,
0.002627694031,0.001294606301,0,-0.00106585844,-0.001780704474,-0.00209829715,-0.002042805529,-0.001692676233,-0.001158372906,-0.0005587126577,0,0.000439005601,0.0007147569137,0.000819218344,0.0007740754133,0.0006209758261,0.0004102492819,0.0001903855486,0,-0.0001367537005,-0.0002111746303,-0.0002279542535,
-0.0002010766891,-0.000148882116,-0.000089420931270978,-0.0000369464911936416,0
};


const float polyphaseFIRcoefs_10[240] = {0,-0.0000281504431348934,-0.0000669762387258759,-0.0001130499027,-0.0001604431498,-0.0002010766891,-0.0002255154872,-0.0002241765536,-0.0001888448401,-0.0001143246627,0,0.0001489524525,0.0003209500834,0.0004982022552,0.0006578511505,0.0007740754133,0.0008210425382,0.0007764557284,0.0006253130798,0.0003634012087,
0,-0.0004407060652,-0.0009199244125,-0.00138651315,-0.001781237884,-0.002042805529,-0.002115187499,-0.001955502112,-0.001541542335,-0.0008779424399,0,0.001025679059,0.002104267035,0.003119701633,0.003945283421,0.004457108117,0.004549104717,0.004148088598,0.003227024759,0.001814674971,0,-0.002069893334,-0.004198854707,
-0.006157675118,-0.007705953084,-0.008618027479,-0.008710467394,-0.007868129332,-0.006065607616,-0.003381069394,0,0.003792735029,0.007632943749,0.0111083731,0.01379890525,0.01532214346,0.01537985699,0.01380017716,0.01057038006,0.005855610849,0,-0.006491775967,-0.01299245922,-0.01880750406,-0.02324337119,-0.0256826399,
-0.02565840841,-0.02291980874,-0.01748065659,-0.009644341665,0,0.01061234095,0.02116681726,0.03054283259,0.03763489182,0.04147118748,0.04132914257,0.03683539611,0.0280382689,0.01544260921,0,-0.01694861966,-0.03377615484,-0.04871170541,-0.06001078807,-0.06613834255,-0.06594687843,-0.05883116731,-0.0448417517,-0.02474226599,
0,0.02729504767,0.05458067226,0.07903357958,0.09782558164,0.1084038132,0.1087700655,0.0977326428,0.07510489725,0.04182786277,0,-0.0471954136,-0.09566588962,-0.1406796077,-0.1772117996,-0.2003420047,-0.205670663,-0.1897202018,-0.1502854689,-0.08670119472,0,0.1070571509,0.2300858789,0.3633152572,0.4999418792,0.6325760967,
0.7537470349,0.8564273523,0.9345363343,0.9833810618,1,0.9833810618,0.9345363343,0.8564273523,0.7537470349,0.6325760967,0.4999418792,0.3633152572,0.2300858789,0.1070571509,0,-0.08670119472,-0.1502854689,-0.1897202018,-0.205670663,-0.2003420047,-0.1772117996,-0.1406796077,-0.09566588962,-0.0471954136,0,0.04182786277,
0.07510489725,0.0977326428,0.1087700655,0.1084038132,0.09782558164,0.07903357958,0.05458067226,0.02729504767,0,-0.02474226599,-0.0448417517,-0.05883116731,-0.06594687843,-0.06613834255,-0.06001078807,-0.04871170541,-0.03377615484,-0.01694861966,0,0.01544260921,0.0280382689,0.03683539611,0.04132914257,0.04147118748,
0.03763489182,0.03054283259,0.02116681726,0.01061234095,0,-0.009644341665,-0.01748065659,-0.02291980874,-0.02565840841,-0.0256826399,-0.02324337119,-0.01880750406,-0.01299245922,-0.006491775967,0,0.005855610849,0.01057038006,0.01380017716,0.01537985699,0.01532214346,0.01379890525,0.0111083731,0.007632943749,0.003792735029,
0,-0.003381069394,-0.006065607616,-0.007868129332,-0.008710467394,-0.008618027479,-0.007705953084,-0.006157675118,-0.004198854707,-0.002069893334,0,0.001814674971,0.003227024759,0.004148088598,0.004549104717,0.004457108117,0.003945283421,0.003119701633,0.002104267035,0.001025679059,0,-0.0008779424399,-0.001541542335,
-0.001955502112,-0.002115187499,-0.002042805529,-0.001781237884,-0.00138651315,-0.0009199244125,-0.0004407060652,0,0.0003634012087,0.0006253130798,0.0007764557284,0.0008210425382,0.0007740754133,0.0006578511505,0.0004982022552,0.0003209500834,0.0001489524525,0,-0.0001143246627,-0.0001888448401,-0.0002241765536,
-0.0002255154872,-0.0002010766891,-0.0001604431498,-0.0001130499027,-0.0000669762387258759,-0.0000281504431348934
};

const float polyphaseFIRcoefs_10_reversed[240] = {-0.0000281504431348934,-0.0000669762387258759,-0.0001130499027,-0.0001604431498,-0.0002010766891,-0.0002255154872,-0.0002241765536,-0.0001888448401,-0.0001143246627,0,0.0001489524525,0.0003209500834,0.0004982022552,0.0006578511505,0.0007740754133,0.0008210425382,0.0007764557284,0.0006253130798,0.0003634012087,
0,-0.0004407060652,-0.0009199244125,-0.00138651315,-0.001781237884,-0.002042805529,-0.002115187499,-0.001955502112,-0.001541542335,-0.0008779424399,0,0.001025679059,0.002104267035,0.003119701633,0.003945283421,0.004457108117,0.004549104717,0.004148088598,0.003227024759,0.001814674971,0,-0.002069893334,-0.004198854707,
-0.006157675118,-0.007705953084,-0.008618027479,-0.008710467394,-0.007868129332,-0.006065607616,-0.003381069394,0,0.003792735029,0.007632943749,0.0111083731,0.01379890525,0.01532214346,0.01537985699,0.01380017716,0.01057038006,0.005855610849,0,-0.006491775967,-0.01299245922,-0.01880750406,-0.02324337119,-0.0256826399,
-0.02565840841,-0.02291980874,-0.01748065659,-0.009644341665,0,0.01061234095,0.02116681726,0.03054283259,0.03763489182,0.04147118748,0.04132914257,0.03683539611,0.0280382689,0.01544260921,0,-0.01694861966,-0.03377615484,-0.04871170541,-0.06001078807,-0.06613834255,-0.06594687843,-0.05883116731,-0.0448417517,-0.02474226599,
0,0.02729504767,0.05458067226,0.07903357958,0.09782558164,0.1084038132,0.1087700655,0.0977326428,0.07510489725,0.04182786277,0,-0.0471954136,-0.09566588962,-0.1406796077,-0.1772117996,-0.2003420047,-0.205670663,-0.1897202018,-0.1502854689,-0.08670119472,0,0.1070571509,0.2300858789,0.3633152572,0.4999418792,0.6325760967,
0.7537470349,0.8564273523,0.9345363343,0.9833810618,1,0.9833810618,0.9345363343,0.8564273523,0.7537470349,0.6325760967,0.4999418792,0.3633152572,0.2300858789,0.1070571509,0,-0.08670119472,-0.1502854689,-0.1897202018,-0.205670663,-0.2003420047,-0.1772117996,-0.1406796077,-0.09566588962,-0.0471954136,0,0.04182786277,
0.07510489725,0.0977326428,0.1087700655,0.1084038132,0.09782558164,0.07903357958,0.05458067226,0.02729504767,0,-0.02474226599,-0.0448417517,-0.05883116731,-0.06594687843,-0.06613834255,-0.06001078807,-0.04871170541,-0.03377615484,-0.01694861966,0,0.01544260921,0.0280382689,0.03683539611,0.04132914257,0.04147118748,
0.03763489182,0.03054283259,0.02116681726,0.01061234095,0,-0.009644341665,-0.01748065659,-0.02291980874,-0.02565840841,-0.0256826399,-0.02324337119,-0.01880750406,-0.01299245922,-0.006491775967,0,0.005855610849,0.01057038006,0.01380017716,0.01537985699,0.01532214346,0.01379890525,0.0111083731,0.007632943749,0.003792735029,
0,-0.003381069394,-0.006065607616,-0.007868129332,-0.008710467394,-0.008618027479,-0.007705953084,-0.006157675118,-0.004198854707,-0.002069893334,0,0.001814674971,0.003227024759,0.004148088598,0.004549104717,0.004457108117,0.003945283421,0.003119701633,0.002104267035,0.001025679059,0,-0.0008779424399,-0.001541542335,
-0.001955502112,-0.002115187499,-0.002042805529,-0.001781237884,-0.00138651315,-0.0009199244125,-0.0004407060652,0,0.0003634012087,0.0006253130798,0.0007764557284,0.0008210425382,0.0007740754133,0.0006578511505,0.0004982022552,0.0003209500834,0.0001489524525,0,-0.0001143246627,-0.0001888448401,-0.0002241765536,
-0.0002255154872,-0.0002010766891,-0.0001604431498,-0.0001130499027,-0.0000669762387258759,-0.0000281504431348934,0
};

const float polyphaseFIRcoefs_12[288] = {0,-0.0000226618432342336,-0.0000530072485187154,-0.000089420931270978,-0.0001290614862,-0.0001679283668,-0.0002010766891,-0.0002229819002,-0.0002280403163,-0.0002111746303,-0.0001684973702,-9.80E-05,0,0.0001221336873,0.000262035748,0.0004102492819,0.0005546137386,0.0006809528516,0.0007740754133,0.0008190406621,0.0008026074303,
0.0007147569137,0.0005501562179,0.0003094166991,0,-0.0003633628239,-0.0007588470574,-0.001158372906,-0.001529123503,-0.001835699621,-0.002042805529,-0.002118299361,-0.002036387807,-0.001780704474,-0.001346989195,-0.0007450856826,0,0.000848187327,0.001745688527,0.002627694031,0.00342230706,0.004055544231,0.004457108117,
0.004566539717,0.00433928347,0.003752153484,0.002807684058,0.001536884609,0,-0.001715000455,-0.003496324479,-0.005214584823,-0.006731093724,-0.007907769784,-0.008618027479,-0.00875787804,-0.008256378144,-0.007084531631,-0.005261787503,-0.002859387882,0,0.003146691743,0.006372467967,0.009442846969,0.01211254026,0.01414317934,
0.01532214346,0.0154811292,0.0145130044,0.01238549626,0.00915038426,0.004947103168,0,-0.005391086448,-0.01086677004,-0.01602992044,-0.02047215835,-0.02380342436,-0.0256826399,-0.0258472382,-0.02413925292,-0.02052573092,-0.01511148857,-0.008142652993,0,0.008818212476,0.01772345683,0.02607295427,0.03321251426,0.03852355813,
0.04147118748,0.0416498511,0.03882308985,0.03295401713,0.02422362826,0.01303470915,0,-0.01408568736,-0.0282885659,-0.04159220087,-0.05296400188,-0.06142787805,-0.06613834255,-0.06645093005,-0.06198370246,-0.05266489232,-0.03876237002,-0.02089159771,0,0.02267283264,0.045658078,0.06734108688,0.08606104175,0.1002213015,0.1084038132,
0.1094803065,0.1027127741,0.08783600994,0.06511574397,0.03537713351,0,-0.03912086292,-0.07964984885,-0.1189165043,-0.1540485155,-0.1821250217,-0.2003420047,-0.2061803997,-0.197567026,-0.1730184801,-0.1317587985,-0.07380294983,0,0.08796799709,0.187632816,0.2958313541,0.4088375251,0.5225283984,0.6325760967,0.734655299,0.8246551151,
0.898883675,0.954254044,0.988441027,1,0.988441027,0.954254044,0.898883675,0.8246551151,0.734655299,0.6325760967,0.5225283984,0.4088375251,0.2958313541,0.187632816,0.08796799709,0,-0.07380294983,-0.1317587985,-0.1730184801,-0.197567026,-0.2061803997,-0.2003420047,-0.1821250217,-0.1540485155,-0.1189165043,-0.07964984885,-0.03912086292,
0,0.03537713351,0.06511574397,0.08783600994,0.1027127741,0.1094803065,0.1084038132,0.1002213015,0.08606104175,0.06734108688,0.045658078,0.02267283264,0,-0.02089159771,-0.03876237002,-0.05266489232,-0.06198370246,-0.06645093005,-0.06613834255,-0.06142787805,-0.05296400188,-0.04159220087,-0.0282885659,-0.01408568736,0,0.01303470915,
0.02422362826,0.03295401713,0.03882308985,0.0416498511,0.04147118748,0.03852355813,0.03321251426,0.02607295427,0.01772345683,0.008818212476,0,-0.008142652993,-0.01511148857,-0.02052573092,-0.02413925292,-0.0258472382,-0.0256826399,-0.02380342436,-0.02047215835,-0.01602992044,-0.01086677004,-0.005391086448,0,0.004947103168,
0.00915038426,0.01238549626,0.0145130044,0.0154811292,0.01532214346,0.01414317934,0.01211254026,0.009442846969,0.006372467967,0.003146691743,0,-0.002859387882,-0.005261787503,-0.007084531631,-0.008256378144,-0.00875787804,-0.008618027479,-0.007907769784,-0.006731093724,-0.005214584823,-0.003496324479,-0.001715000455,0,0.001536884609,
0.002807684058,0.003752153484,0.00433928347,0.004566539717,0.004457108117,0.004055544231,0.00342230706,0.002627694031,0.001745688527,0.000848187327,0,-0.0007450856826,-0.001346989195,-0.001780704474,-0.002036387807,-0.002118299361,-0.002042805529,-0.001835699621,-0.001529123503,-0.001158372906,-0.0007588470574,-0.0003633628239,
0,0.0003094166991,0.0005501562179,0.0007147569137,0.0008026074303,0.0008190406621,0.0007740754133,0.0006809528516,0.0005546137386,0.0004102492819,0.000262035748,0.0001221336873,0,-9.80E-05,-0.0001684973702,-0.0002111746303,-0.0002280403163,-0.0002229819002,-0.0002010766891,-0.0001679283668,-0.0001290614862,
-0.000089420931270978,-0.0000530072485187154,-0.0000226618432342336
};

const float polyphaseFIRcoefs_12_reversed[288] = {-0.0000226618432342336,-0.0000530072485187154,-0.000089420931270978,-0.0001290614862,-0.0001679283668,-0.0002010766891,-0.0002229819002,-0.0002280403163,-0.0002111746303,-0.0001684973702,-9.80E-05,0,0.0001221336873,0.000262035748,0.0004102492819,0.0005546137386,0.0006809528516,0.0007740754133,0.0008190406621,0.0008026074303,
0.0007147569137,0.0005501562179,0.0003094166991,0,-0.0003633628239,-0.0007588470574,-0.001158372906,-0.001529123503,-0.001835699621,-0.002042805529,-0.002118299361,-0.002036387807,-0.001780704474,-0.001346989195,-0.0007450856826,0,0.000848187327,0.001745688527,0.002627694031,0.00342230706,0.004055544231,0.004457108117,
0.004566539717,0.00433928347,0.003752153484,0.002807684058,0.001536884609,0,-0.001715000455,-0.003496324479,-0.005214584823,-0.006731093724,-0.007907769784,-0.008618027479,-0.00875787804,-0.008256378144,-0.007084531631,-0.005261787503,-0.002859387882,0,0.003146691743,0.006372467967,0.009442846969,0.01211254026,0.01414317934,
0.01532214346,0.0154811292,0.0145130044,0.01238549626,0.00915038426,0.004947103168,0,-0.005391086448,-0.01086677004,-0.01602992044,-0.02047215835,-0.02380342436,-0.0256826399,-0.0258472382,-0.02413925292,-0.02052573092,-0.01511148857,-0.008142652993,0,0.008818212476,0.01772345683,0.02607295427,0.03321251426,0.03852355813,
0.04147118748,0.0416498511,0.03882308985,0.03295401713,0.02422362826,0.01303470915,0,-0.01408568736,-0.0282885659,-0.04159220087,-0.05296400188,-0.06142787805,-0.06613834255,-0.06645093005,-0.06198370246,-0.05266489232,-0.03876237002,-0.02089159771,0,0.02267283264,0.045658078,0.06734108688,0.08606104175,0.1002213015,0.1084038132,
0.1094803065,0.1027127741,0.08783600994,0.06511574397,0.03537713351,0,-0.03912086292,-0.07964984885,-0.1189165043,-0.1540485155,-0.1821250217,-0.2003420047,-0.2061803997,-0.197567026,-0.1730184801,-0.1317587985,-0.07380294983,0,0.08796799709,0.187632816,0.2958313541,0.4088375251,0.5225283984,0.6325760967,0.734655299,0.8246551151,
0.898883675,0.954254044,0.988441027,1,0.988441027,0.954254044,0.898883675,0.8246551151,0.734655299,0.6325760967,0.5225283984,0.4088375251,0.2958313541,0.187632816,0.08796799709,0,-0.07380294983,-0.1317587985,-0.1730184801,-0.197567026,-0.2061803997,-0.2003420047,-0.1821250217,-0.1540485155,-0.1189165043,-0.07964984885,-0.03912086292,
0,0.03537713351,0.06511574397,0.08783600994,0.1027127741,0.1094803065,0.1084038132,0.1002213015,0.08606104175,0.06734108688,0.045658078,0.02267283264,0,-0.02089159771,-0.03876237002,-0.05266489232,-0.06198370246,-0.06645093005,-0.06613834255,-0.06142787805,-0.05296400188,-0.04159220087,-0.0282885659,-0.01408568736,0,0.01303470915,
0.02422362826,0.03295401713,0.03882308985,0.0416498511,0.04147118748,0.03852355813,0.03321251426,0.02607295427,0.01772345683,0.008818212476,0,-0.008142652993,-0.01511148857,-0.02052573092,-0.02413925292,-0.0258472382,-0.0256826399,-0.02380342436,-0.02047215835,-0.01602992044,-0.01086677004,-0.005391086448,0,0.004947103168,
0.00915038426,0.01238549626,0.0145130044,0.0154811292,0.01532214346,0.01414317934,0.01211254026,0.009442846969,0.006372467967,0.003146691743,0,-0.002859387882,-0.005261787503,-0.007084531631,-0.008256378144,-0.00875787804,-0.008618027479,-0.007907769784,-0.006731093724,-0.005214584823,-0.003496324479,-0.001715000455,0,0.001536884609,
0.002807684058,0.003752153484,0.00433928347,0.004566539717,0.004457108117,0.004055544231,0.00342230706,0.002627694031,0.001745688527,0.000848187327,0,-0.0007450856826,-0.001346989195,-0.001780704474,-0.002036387807,-0.002118299361,-0.002042805529,-0.001835699621,-0.001529123503,-0.001158372906,-0.0007588470574,-0.0003633628239,
0,0.0003094166991,0.0005501562179,0.0007147569137,0.0008026074303,0.0008190406621,0.0007740754133,0.0006809528516,0.0005546137386,0.0004102492819,0.000262035748,0.0001221336873,0,-9.80E-05,-0.0001684973702,-0.0002111746303,-0.0002280403163,-0.0002229819002,-0.0002010766891,-0.0001679283668,-0.0001290614862,
-0.000089420931270978,-0.0000530072485187154,-0.0000226618432342336,0
};