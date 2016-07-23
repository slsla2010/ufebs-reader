//////////  Правила обработки ED101 /////////////////////////
struct rule_descr rules[8] = {
 { ":Payee:PersonalAcc" , "40[5-8]0[2-9]" , _WRITE_ED101_ , NULL }  ,
 { ":Payee:PersonalAcc" , "408[12][027]" , _SKIP_ED101_ , NULL }  ,
 { ":Payee:PersonalAcc" , "455[0-9]" , _WRITE_ED101_ , NULL }  ,
 { ":Payee:PersonalAcc" , "4230[1-6]"    , _SKIP_ED101_ , NULL }  ,
 { ":Payee:PersonalAcc" , "47422"    , _SKIP_ED101_ , NULL }  ,
 { ":Payee:Bank:BIC"    , "048073601"     , _SKIP_ED101_ , NULL },{NULL,NULL,0,NULL}} ;
////////////////////////////////////////////////////////////////
