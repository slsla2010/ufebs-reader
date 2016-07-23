# ufebs-reader
read&amp;write ufebs file Bank Russia
01/06/2015
 ключ запуска -F FULLMENU

12/05/2015
 #define FULL_MENU
 если не определить - то менюшки в сокращенном виде
 в Main.h блок
	// параметры сборки разных версий
	//#define MULTI_DOC
	//#define TEST_CTL3D	
	//#define FULL_MENU



11/05/2015 
  добавил в тулбар "Кодировать в B64" и "Декодировать из B64"

10/05/2015
  Операция "анализ УПД" теперь работает по регулярным маскам из rules.h ( XPATH+REGEX ) 
  ранее тупо сравнивала счет получателя PersonalAcc с 423 и 40817 

07.05.2015
 - доработано логирование действий
 - лог теперь пишется в уникальный файл типа ur_DD-MM-YYYY.log
 - убрал комментирование действий из заголовка окна
 - теперь в заголовке имя загруженного файла
 - флаги настроек переведены в единый массив и чекаются не по одиночке, а в цикле


21.04.2015
 меню настроек-блок Выборок теперь строится динамически
 из rules.h
 //TODO  конфиг rules.h подменять из командной строки, так же как в chdatexml 


16.04.2015
 Добавлен блок настроек для XML
 Можно проставить крыжи "Удалять 40817" , "Удалять 423"
 Если крыжи проставлены , то операция "Выборка ED101" cоответственно удалит платежки
 в которых PersonalAcc проставлен в 40817 либо 423


01.04.2015
 Добавлена операция "Выделить SWIFTDocument(ED503)" для просмотра ED503
 Показывает первый SWIFTDocument из файла ( таких документов может быть несколько,
    но сейчас программа показывает первый из них)
 Можно использовать для просмотра , но запускать 
 с командной строки надо с ключом -0
    ur -0 ED501XXXX.ED
 иначе крэш на проверке ED501
 



03.03.2015

Добавлена операция "Пересборка PacketEPD"

  При выполнении операции  пакет разваливается на отдельные ED101, затем собирается 

  в новый используя  правила из файла  rules.h 



//////////  Правила обработки ED101 /////////////////////////

struct rule_descr rules[8] = {

 { ":Payee:PersonalAcc" , "40817810[0-9]06" , _WRITE_ED101_ , NULL }  ,

 { ":Payee:PersonalAcc" , "408[12][027]"      , _SKIP_ED101_ , NULL }  ,

 { ":Payer:PersonalAcc" , "40702810[0-9]0634" , _WRITE_ED101_ , NULL }  ,

 { ":Payee:PersonalAcc" , "42[34]0"       , _SKIP_ED101_ , NULL }  ,

 { ":Payee:Bank:BIC"    , "048073601"     , _SKIP_ED101_ , NULL },{NULL,NULL,0,NULL}} ;

////////////////////////////////////////////////////////////////



То есть в новый пакет включаются только платежки, которые попадают 

под правило WRITE, а те что попадают под SKIP - игнорируются и в новый 

пакет не включаются

Таким образом строится пакет не содержащий платежки которые могут пойти на ЦОД







25.02.2015
   Добавлен пункт "Xpath поиск"
   Можно ввести XPATH выражения типа 
      "/my:PacketEPD/my:ED101[@EDAuthor="7195753000"]"
   или 
      /my:PacketEPD/my:ED101/my:Payee[@PersonalAcc="40802810017000180048"]
   и получить сколько данных документов найдено в пакете
   AHTUNG! если XPATH построен некорректно будет крэш







18.02.2015
разбор параметров командной строки для установки глобальных флагов

ПРИ ОТКРЫТИИ
int   gF_checkED501      = 1 ; // при открытии проверять не ED501 ли это


ПРИ ЗАПИСИ
int   gF_compressSpaces  = 1 ; // при кодировании в BASE64 удалять переводы строки, последовательности пробелов
int   gF_changeEDNo      = 0 ; // предварительно поменять все тэги EDNo
int   gF_changeAccDocNo  = 0 ; // предварительно поменять все тэги AccDocNo
int   gF_recalculate     = 0 ; // перерасчет количества EDQuantity и итоговой суммы документов 
int   gF_saveProper      = 1 ; // сразу сохранить файл с правильным именем при
			       // запуске блока "Установить новую дату и упаковать"

			case 'E': gF_changeEDNo      = 1 ;break;
			case 'e': gF_changeEDNo      = 0 ;break;

			case 'B': gF_compressSpaces  = 1 ;break;
			case 'b': gF_compressSpaces  = 0 ;break;

			case 'A': gF_changeAccDocNo  = 1 ;break;
			case 'a': gF_changeAccDocNo  = 0 ;break;

			case 'R': gF_recalculate     = 1 ;break;
			case 'r': gF_recalculate     = 0 ;break;
			case 'S': gF_saveProper      = 1 ;break;
			case 's': gF_saveProper      = 0 ;break;
			case '5': gF_checkED501      = 1 ;break;
			case '0': gF_checkED501      = 0 ;break;




16.02.2015
 добавлен функционал распознавания ED501
 при запуске с командной строки
 программа после распаковки попытается определить - не ED501 ли это ?
 если ED501 - то выделит и распакует ProprietoryDocument
 работает через xpath - запускать только на корректных XML 
 на простом тексте будет крэш





16.01.2015
 добавлен функционал анализа на документы предположительно 
 смаршрутизирующиеся на УПД 
 работает через xpath - запускать только на корректных XML 
 на простом тексте будет крэш







21.12.14
 Добавлен функционал пересчета количества и суммы в PacketEPD
 меняет тэги EDQuantity и Sum
 работает через xpath

 создает в текущем каталоге временный файл !bout.txt,если нет прав - будет 
 ошибка.
 использовать с осторожностью




VVIvanenkov.SBT@sberbank.ru







MDcuMDUuMjAxNQ0KIC0g5O7w4OHu8uDt7iDr7uPo8O7i4O3o5SDk5enx8uLo6Q0KIC0g6+7jIPLl7+Xw/CDv6Pjl8vH/IOIg8+3o6uDr/O376SD04OnrIPLo7+AgdXJfREQtTU0tWVlZWS5sb2cNCiAtIPPh8ODrIOru7Ozl7fLo8O7i4O3o5SDk5enx8uLo6SDo5yDn4OPu6+7i6uAg7urt4A0KIC0g8uXv5fD8IOIg5+Dj7uvu4urlIOjs/yDn4OPw8+bl7e3u4+4g9ODp6+ANCiAtIPTr4OPoIO3g8fLw7uXqIO/l8OXi5eTl7fsg4iDl5Ojt++kg7ODx8ejiIOgg9+Xq4P7y8f8g7eUg7+4g7uTo7e736uUsIOAg4iD26Orr5Q0KDQoNCjIxLjA0LjIwMTUNCiDs5e3+IO3g8fLw7uXqLeHr7uogwvvh7vDu6iDy5e/l8Pwg8fLw7ujy8f8g5Ojt4Ozo9+Xx6ugNCiDo5yBydWxlcy5oDQogLy9UT0RPICDq7u306OMgcnVsZXMuaCDv7uTs5e3/8vwg6Ocg6u7s4O3k7e7pIPHy8O7q6Cwg8uDqIOblIOrg6iDiIGNoZGF0ZXhtbCANCg0KDQoxNi4wNC4yMDE1DQogxO7h4OLr5e0g4evu6iDt4PHy8O7l6iDk6/8gWE1MDQogzO7m7e4g7/Du8fLg4ujy/CDq8Pvm6CAi0+Tg6//y/CA0MDgxNyIgLCAi0+Tg6//y/CA0MjMiDQogxfHr6CDq8Pvm6CDv8O7x8uDi6+Xt+yAsIPLuIO7v5fDg9uj/ICLC++Hu8OrgIEVEMTAxIiBj7u7y4uXy8fLi5e3t7iDz5ODr6PIg7+vg8uXm6ugNCiDiIOru8u7w+/UgUGVyc29uYWxBY2Mg7/Du8fLg4uvl7SDiIDQwODE3IOvo4e4gNDIzDQoNCg0KMDEuMDQuMjAxNQ0KIMTu4eDi6+Xt4CDu7+Xw4Pbo/yAiwvvk5evo8vwgU1dJRlREb2N1bWVudChFRDUwMykiIOTr/yDv8O7x7O7y8OAgRUQ1MDMNCiDP7urg5/vi4OXyIO/l8OL76SBTV0lGVERvY3VtZW50IOjnIPTg6evgICgg8uDq6PUg5O7q8+zl7fLu4iDs7ubl8iDh+/L8IO3l8eru6/zq7iwNCiAgICDt7iDx5en34PEg7/Du4/Dg7OzgIO/u6uDn++Lg5fIg7+Xw4vvpIOjnIO3o9SkNCiDM7ubt7iDo8e/u6/zn7uLg8vwg5Ov/IO/w7vHs7vLw4CAsIO3uIOfg7/Px6uDy/CDxIOru7ODt5O3u6SDx8vDu6ugg7eDk7iDxIOrr/vfu7CAtMA0KICAgIHVyIC0wIEVENTAxWFhYWC5FRA0KIOjt4PflIOrw/fgg7eAg7/Du4uXw6uUgRUQ1MDENCiANCg0KDQ0KMDMuMDMuMjAxNQ0NCsTu4eDi6+Xt4CDu7+Xw4Pbo/yAiz+Xw5fHh7vDq4CBQYWNrZXRFUEQiDQ0KICDP8Ogg4vvv7uvt5e3o6CDu7+Xw4Pbo6CAg7+Dq5fIg8ODn4uDr6OLg5fLx/yDt4CDu8uTl6/zt++UgRUQxMDEsIOfg8uXsIPHu4ejw4OXy8f8gDQ0KICDiIO3u4vvpIOjx7+7r/Ofz/yAg7/Dg4ujr4CDo5yD04Onr4CAgcnVsZXMuaCANDQoNDQovLy8vLy8vLy8vICDP8ODi6OvgIO7h8ODh7vLq6CBFRDEwMSAvLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vDQ0Kc3RydWN0IHJ1bGVfZGVzY3IgcnVsZXNbOF0gPSB7DQ0KIHsgIjpQYXllZTpQZXJzb25hbEFjYyIgLCAiNDA4MTc4MTBbMC05XTA2IiAsIF9XUklURV9FRDEwMV8gLCBOVUxMIH0gICwNDQogeyAiOlBheWVlOlBlcnNvbmFsQWNjIiAsICI0MDhbMTJdWzAyN10iICAgICAgLCBfU0tJUF9FRDEwMV8gLCBOVUxMIH0gICwNDQogeyAiOlBheWVyOlBlcnNvbmFsQWNjIiAsICI0MDcwMjgxMFswLTldMDYzNCIgLCBfV1JJVEVfRUQxMDFfICwgTlVMTCB9ICAsDQ0KIHsgIjpQYXllZTpQZXJzb25hbEFjYyIgLCAiNDJbMzRdMCIgICAgICAgLCBfU0tJUF9FRDEwMV8gLCBOVUxMIH0gICwNDQogeyAiOlBheWVlOkJhbms6QklDIiAgICAsICIwNDgwNzM2MDEiICAgICAsIF9TS0lQX0VEMTAxXyAsIE5VTEwgfSx7TlVMTCxOVUxMLDAsTlVMTH19IDsNDQovLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vLy8vDQ0KDQ0K0u4g5fHy/CDiIO3u4vvpIO/g6uXyIOLq6/734P7y8f8g8u7r/OruIO/r4PLl5uroLCDq7vLu8PvlIO/u7+Dk4P7yIA0NCu/u5CDv8ODi6OvuIFdSSVRFLCDgIPLlIPfy7iDv7u/g5OD+8iDv7uQgU0tJUCAtIOjj7e7w6PDz/vLx/yDoIOIg7e7i++kgDQ0K7+Dq5fIg7eUg4urr/vfg/vLx/w0NCtLg6ujsIO7h8ODn7uwg8fLw7ujy8f8g7+Dq5fIg7eUg8e7k5fDm4Pno6SDv6+Dy5ebq6CDq7vLu8PvlIOzu4/PyIO/u6fLoIO3gINbOxA0NCg0NCg0NCg0NCjI1LjAyLjIwMTUNICAgxO7h4OLr5e0g7/Pt6vIgIlhwYXRoIO/u6PHqIg0gICDM7ubt7iDi4uXx8uggWFBBVEgg4vvw4Obl7ej/IPLo7+AgDSAgICAgICIvbXk6UGFja2V0RVBEL215OkVEMTAxW0BFREF1dGhvcj0iNzE5NTc1MzAwMCJdIg0gICDo6+ggDSAgICAgIC9teTpQYWNrZXRFUEQvbXk6RUQxMDEvbXk6UGF5ZWVbQFBlcnNvbmFsQWNjPSI0MDgwMjgxMDAxNzAwMDE4MDA0OCJdDSAgIOgg7+7r8/fo8vwg8eru6/zq7iDk4O3t+/Ug5O7q8+zl7fLu4iDt4Onk5e3uIOIg7+Dq5fLlDSAgIEFIVFVORyEg5fHr6CBYUEFUSCDv7vHy8O7l7SDt5eru8PDl6vLt7iDh8+Tl8iDq8P34DQ0KDQ0KDQ0KDQ0KMTguMDIuMjAxNQ0K8ODn4e7wIO/g8ODs5fLw7uIg6u7s4O3k7e7pIPHy8O7q6CDk6/8g8/Hy4O3u4uroIOPr7uHg6/zt+/Ug9Ovg4+7iDQoNz9DIIM7SytDb0sjIDWludCAgIGdGX2NoZWNrRUQ1MDEgICAgICA9IDEgOyAvLyDv8Ogg7vLq8Pvy6Ogg7/Du4uXw//L8IO3lIEVENTAxIOvoIP3y7g0NCg3P0Mggx8DPyNHIDWludCAgIGdGX2NvbXByZXNzU3BhY2VzICA9IDEgOyAvLyDv8Ogg6u7k6PDu4uDt6Ogg4iBCQVNFNjQg8+Tg6//y/CDv5fDl4u7k+yDx8vDu6ugsIO/u8evl5O7i4PLl6/zt7vHy6CDv8O7h5evu4g1pbnQgICBnRl9jaGFuZ2VFRE5vICAgICAgPSAwIDsgLy8g7/Dl5OLg8Ojy5ev87e4g7+7s5e3/8vwg4vHlIPL94+ggRURObw1pbnQgICBnRl9jaGFuZ2VBY2NEb2NObyAgPSAwIDsgLy8g7/Dl5OLg8Ojy5ev87e4g7+7s5e3/8vwg4vHlIPL94+ggQWNjRG9jTm8NaW50ICAgZ0ZfcmVjYWxjdWxhdGUgICAgID0gMCA7IC8vIO/l8OXw4PH35fIg6u7r6Pfl8fLi4CBFRFF1YW50aXR5IOgg6PLu4+7i7ukg8fPs7Psg5O7q8+zl7fLu4iANaW50ICAgZ0Zfc2F2ZVByb3BlciAgICAgID0gMSA7IC8vIPHw4OfzIPHu9fDg7ejy/CD04OnrIPEg7/Dg4ujr/O377CDo7OXt5ewg7/DoDQkJCSAgICAgICAvLyDn4O/z8erlIOHr7urgICLT8fLg7e7i6PL8IO3u4vP+IOTg8vMg6CDz7+Dq7uLg8vwiDQ0JCQljYXNlICdFJzogZ0ZfY2hhbmdlRURObyAgICAgID0gMSA7YnJlYWs7DQkJCWNhc2UgJ2UnOiBnRl9jaGFuZ2VFRE5vICAgICAgPSAwIDticmVhazsNDQkJCWNhc2UgJ0InOiBnRl9jb21wcmVzc1NwYWNlcyAgPSAxIDticmVhazsNCQkJY2FzZSAnYic6IGdGX2NvbXByZXNzU3BhY2VzICA9IDAgO2JyZWFrOw0NCQkJY2FzZSAnQSc6IGdGX2NoYW5nZUFjY0RvY05vICA9IDEgO2JyZWFrOw0JCQljYXNlICdhJzogZ0ZfY2hhbmdlQWNjRG9jTm8gID0gMCA7YnJlYWs7DQ0JCQljYXNlICdSJzogZ0ZfcmVjYWxjdWxhdGUgICAgID0gMSA7YnJlYWs7DQkJCWNhc2UgJ3InOiBnRl9yZWNhbGN1bGF0ZSAgICAgPSAwIDticmVhazsNCQkJY2FzZSAnUyc6IGdGX3NhdmVQcm9wZXIgICAgICA9IDEgO2JyZWFrOw0JCQljYXNlICdzJzogZ0Zfc2F2ZVByb3BlciAgICAgID0gMCA7YnJlYWs7DQkJCWNhc2UgJzUnOiBnRl9jaGVja0VENTAxICAgICAgPSAxIDticmVhazsNCQkJY2FzZSAnMCc6IGdGX2NoZWNrRUQ1MDEgICAgICA9IDAgO2JyZWFrOw0NDQoNDQoxNi4wMi4yMDE1DQog5O7h4OLr5e0g9PPt6vbo7u3g6yDw4PHv7uft4OLg7ej/IEVENTAxDQog7/DoIOfg7/Px6uUg8SDq7uzg7eTt7ukg8fLw7uroDQog7/Du4/Dg7OzgIO/u8evlIPDg8e/g6u7i6ugg7+7v+/Lg5fLx/yDu7/Dl5OXr6PL8IC0g7eUgRUQ1MDEg6+gg/fLuID8NCiDl8evoIEVENTAxIC0g8u4g4vvk5evo8iDoIPDg8e/g6vPl8iBQcm9wcmlldG9yeURvY3VtZW50DQog8ODh7vLg5fIg9+Xw5ecgeHBhdGggLSDn4O/z8erg8vwg8u7r/OruIO3gIOru8PDl6vLt+/UgWE1MIA0KIO3gIO/w7vHy7uwg8uXq8fLlIOHz5OXyIOrw/fgNCg0NCg0NCg0KMTYuMDEuMjAxNQ0KIOTu4eDi6+XtIPTz7er26O7t4Osg4O3g6+jn4CDt4CDk7urz7OXt8vsg7/Dl5O/u6+7m6PLl6/zt7iANCiDx7ODw+PDz8ujn6PDz/vno5fH/IO3gINPPxCANCiDw4OHu8uDl8iD35fDl5yB4cGF0aCAtIOfg7/Px6uDy/CDy7uv86u4g7eAg6u7w8OXq8u379SBYTUwgDQog7eAg7/Du8fLu7CDy5erx8uUg4fPk5fIg6vD9+A0KDQoNDQoNDQoNDQoyMS4xMi4xNA0KIMTu4eDi6+XtIPTz7er26O7t4Osg7+Xw5fH35fLgIOru6+j35fHy4uAg6CDx8+zs+yDiIFBhY2tldEVQRA0KIOzl7f/l8iDy/ePoIEVEUXVhbnRpdHkg6CBTdW0NCiDw4OHu8uDl8iD35fDl5yB4cGF0aA0KDQog8e7n5ODl8iDiIPLl6vP55ewg6uDy4Ovu4+Ug4vDl7OXt7fvpIPTg6esgIWJvdXQudHh0LOXx6+gg7eXyIO/w4OIgLSDh8+Tl8iANCiDu+Ojh6uAuDQog6PHv7uv85+7i4PL8IPEg7vHy7vDu5u3u8fL8/g0KDQ0KDQ0KVlZJdmFuZW5rb3YuU0JUQHNiZXJiYW5rLnJ1AA==
