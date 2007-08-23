# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl Business-KontoCheck.t'

use Test::More tests => 1021;
BEGIN { use_ok('Business::KontoCheck') };

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

ok(kto_check("10010010","73395105","") eq 1,"Test 1");
ok(kto_check_str("10010010","73395105","") eq "OK","Test 2");
ok(kto_check_at("19800","95556599990","blz-at.lut") eq 2,"Test 1 AT");

while(<DATA>){
   chomp;
   ($typ,$ret,$blz,$kto)=split(/ /);
   if($typ==1){   # deutsche Kontonummern
      ok(kto_check($blz,$kto,"") eq $ret,"BLZ/KTO $blz $kto");
   }
   if($typ==2){   # österreichische Kontonummern
      ok(kto_check_at($blz,$kto,"") eq $ret,"AT BLZ/KTO $blz $kto");
   }
}

__DATA__
1 1 10010010 73395105
1 2 20690500 556343
1 1 25020700 3100755555
1 1 30050000 632935
1 1 36010043 319310430
1 1 37010050 277501502
1 1 37010050 313753507
1 1 37060193 19963012
1 1 38070724 143867000
1 1 40060265 3161600
1 1 50010060 283557609
1 1 50010517 782919930
1 2 50010700 533472
1 1 50190000 500130598
1 1 50850150 639630
1 1 50950068 1440106
1 1 50951469 10324037
1 1 50951469 13116620
1 1 50991400 5004403
1 1 54050220 100984855
1 1 54051990 120006895
1 1 54510067 146201675
1 1 54510067 149311676
1 1 54510067 158480675
1 1 54510067 159025677
1 1 54510067 17764674
1 1 54510067 194067673
1 1 54510067 73144675
1 1 54510067 76910671
1 1 54520194 3930184682
1 1 54540033 2068609
1 1 54550010 8856296
1 1 54550010 978700
1 1 54550120 189233
1 1 54550120 1928183
1 1 54551030 823922
1 1 54560320 2343002
1 1 54561310 815381
1 1 54570024 1546019
1 1 54580020 103790000
1 1 54651240 117127225
1 1 54651240 1406893014
1 1 54651240 380105
1 1 54761411 992526
1 1 54790000 350010
1 1 54851440 20000030
1 1 54851440 26001594
1 1 54851440 26012757
1 1 54851440 26028399
1 1 54862500 45500
1 1 55090500 100161711
1 1 55090500 110581974
1 1 55090500 204568
1 1 55090500 220427
1 1 55090500 3360091
1 1 55090500 581974
1 1 55090500 731516
1 1 55350010 4302379
1 1 55350010 7270086
1 1 58570048 506071
1 1 60010070 276606707
1 1 60090800 1236650
1 1 60090800 204862
1 1 60090800 339515
1 1 60350130 6554
1 1 64390130 11149000
1 1 66010075 142321756
1 1 66010075 321052756
1 1 66010075 354826753
1 1 66010075 363266753
1 1 66010075 382734759
1 1 66010075 94089759
1 1 66050101 10543361
1 1 66090500 333913
1 1 66090500 835455
1 1 66090800 1278797
1 1 66090800 1581058
1 1 66090800 2768836
1 1 66090800 3042871
1 1 66090800 4334671
1 1 66090800 4346327
1 1 66090800 5012287
1 1 67010111 2384856100
1 1 67040031 317450500
1 1 67040031 352029300
1 1 67040031 366096600
1 1 67040031 3782323
1 1 67040031 6060065
1 1 67050101 3136892
1 1 67050101 3152386
1 1 67050101 3245362
1 1 67050101 4055976
1 1 67050101 4199105
1 1 67050101 4298261
1 1 67050101 4704011
1 1 67050101 7218381
1 1 67050101 7566284
1 1 67050101 7692700
1 1 67050101 7863764
1 1 67050101 7882665
1 1 67050101 7890452
1 1 67050101 7951056
1 1 67050505 33168381
1 1 67050505 34006865
1 1 67050505 37381870
1 1 67050505 37904074
1 1 67050505 68010446
1 1 67052385 15184523
1 1 67070010 101063
1 1 67070010 6074066
1 1 67070010 6074454
1 1 67070010 7106289
1 1 67070010 7654312
1 1 67070024 243451
1 1 67070024 6200471
1 1 67070024 7849250
1 1 67080050 4701862900
1 1 67080050 677332100
1 1 67080050 712615300
1 1 74351310 616144
1 1 67080050 741259900
1 1 67080050 744499000
1 1 67090000 2420708
1 1 67090000 2743108
1 1 67090000 3190706
1 1 67090000 3830608
1 1 67090000 4897900
1 1 67090000 5481406
1 1 67090000 6502300
1 1 67090000 779105
1 1 67090000 8449600
1 1 67091300 22219502
1 1 67210111 2080918600
1 1 67250020 25123239
1 1 67250020 3510921
1 1 67250020 4318919
1 1 67270003 454009
1 1 67292200 50767507
1 1 70010080 626687800
1 2 70020270 345661
1 1 70169426 547522
1 1 74351310 616144
1 1 75090300 2152002
1 1 75090300 51799
1 1 76026000 2702164004
1 2 76060000 210850
1 1 76080040 621733300
1 1 82020087 4063503
1 1 44040037 366026300
1 1 54862500 102804026
1 1 54851440 20000444
1 1 20040000 225050400
1 1 46262456 102484800
1 1 54851440 1000033165
1 1 66090800 1486101
1 1 10010010 73245108
1 1 72120078 5724414
1 1 60010070 267474702
1 1 74 239314
1 1 74 239319
1 1 742 0000836601
1 1 742 0000500512
1 1 742 0000857193
1 1 742 0000235374
1 1 742 0000844675
1 1 21050170 844675
1 1 21050170 0000239314
1 2 30020900 1607761319
1 2 70030300 1088774
1 1 70054306 532713
1 1 21566356 110060
1 1 80053762 485065366
1 1 37010050 33323502
1 1 37050299 184004087
1 1 14051462 0069000000
1 1 66050101 22636104
1 1 20050550 1268118377
1 1 54851440 26014035
1 1 16050000 4636004502
1 1 17052302 48074470
1 1 82054052 41010076
1 1 30070010 2508000
1 1 54690623 105186323
1 1 75090300 103002721
1 1 71152570 8737462
1 2 37020500 5379552220
1 1 67050101 4298261 
1 1 67050505 34298262 
1 1 46040033 891695900
1 1 51140029 3700739
1 1 70091600 5147522
1 1 25050180 19968
1 1 60040071 624080805
1 1 60050101 2247067
1 1 51190000 627003
1 1 60010070 267474702
1 1 37060193 122122
1 1 25020600 2323222881
1 1 18062678 4004469
1 1 25010030 904278305
1 1 25010030 763548309
1 1 44010046 266722467
1 1 25010030 749044307
1 1 36080080 429090203
1 1 54851440 26001594
1 1 67090000 2310007
1 1 50010060 106728602
1 1 54790000 20164700
1 1 70070024 531869601
1 1 54510067 6432676
1 1 76010085 78245851
1 1 60040071 523308501
1 1 37010050 285526507
1 1 54510067 216914677
1 1 73150000 247247
1 1 66090800 1486101
1 1 54862500 781355
1 1 55390000 6060005
1 1 36010043 444444431
1 1 37070024 781200100
1 1 44080050 771220000
1 1 75090300 58882
1 1 50010060 301230605
1 1 32250050 236075
1 1 60050101 1018668
1 1 71061009 22284
1 1 70010080 30042806
1 1 54510067 35486679
1 1 54790000 20164700
1 1 20080000 620616500
1 1 87096124 9072772692
1 1 55090500 204568
1 1 67070024 6200471
1 1 54510067 159025677
1 1 67080050 741259900
1 1 67292200 50767507
1 1 67050101 4199105
1 1 55090500 581974
1 1 67050101 4298261
1 1 54551030 823922
1 1 55090500 731516
1 1 66090800 1581058
1 1 54550120 1928183
1 1 38070724 143867000
1 1 67040031 3782323
1 1 54561310 815381
1 1 54550010 978700
1 1 76026000 2702164004
1 1 67070010 6074066
1 1 50951469 10324037
1 1 36010043 319310430
1 1 66010075 94089759
1 1 66090800 5012287
1 1 67050505 34006865
1 1 55090500 100161711
1 1 67250020 25123239
1 1 54520194 3930184682
1 1 66090800 2768836
1 1 54510067 146201675
1 1 67091300 22219502
1 1 67250020 3510921
1 1 67270003 454009
1 1 54510067 76910671
1 1 66010075 354826753
1 1 54550010 8856296
1 1 66090500 835455
1 1 67040031 352029300
1 1 67050101 4055976
1 1 67050101 7890452
1 1 67090000 779105
1 1 67040031 317450500
1 1 67050101 7692700
1 1 67080050 4701862900
1 1 67050505 37381870
1 1 67070010 7106289
1 1 67050101 3136892
1 1 67050101 7218381
1 1 67070010 6074454
1 1 67052385 15184523
1 1 37010050 313753507
1 1 60010070 276606707
1 1 50950068 1440106
1 1 54550120 189233
1 1 67210111 2080918600
1 1 55090500 3695549
1 1 67090000 3190706
1 1 67090000 5481406
1 1 54510067 158480675
1 1 67070010 101063
1 1 67050101 7951056
1 1 67050101 3152386
1 1 67050101 7566284
1 1 67090000 4897900
1 1 67060031 35182500
1 1 67010111 2384856100
1 1 67090000 2420708
1 1 66010075 382734759
1 1 66090500 333913
1 1 10010010 73395105
1 1 66050101 10543361
1 1 67050101 7863764
1 1 50010517 782919930
1 1 67080050 712615300
1 1 54761411 992526
1 1 54540033 2068609
1 1 67090000 2743108
1 1 66090800 4346327
1 1 54651240 117127225
1 1 67050505 37904074
1 1 66010075 363266753
1 1 66090800 4334671
1 1 50190000 500130598
1 1 60090800 339515
1 2 50010700 533472
1 1 54510067 149311676
1 1 66090800 1278797
1 1 67050505 68010446
1 1 55350010 4302379
1 1 67050505 34704015
1 1 58570048 506071
1 1 67050101 3245362
1 1 67080050 744499000
1 1 67040031 366096600
1 1 67070024 7849250
1 1 67090000 3830608
1 1 54651240 1406893014
1 1 67070024 243451
1 1 67070010 7654312
1 1 54570024 1546019
1 1 50991400 5004403
1 1 66090800 3042871
1 1 67040031 6060065
1 1 54580020 103790000
1 1 54510067 73144675
1 1 54050220 100984855
1 1 50951469 13116620
1 1 54510067 194067673
1 1 67090000 8449600
1 1 67050505 33168381
1 1 60090800 204862
1 1 55350010 7270086
1 1 82020087 4063503
1 1 54560320 2343002
1 1 74351310 616144
1 1 37010050 277501502
1 1 67090000 6502300
1 1 37010050 38522505
1 1 67050505 73201888
1 1 54651240 1009510635
1 1 54850010 135062560
1 1 66090800 3330036
1 1 50991400 4138406
1 1 67090000 760307
1 1 37010050 491830508
1 1 67230000 4039634044
1 1 54850010 135302842
1 1 54550120 139691
1 1 66090800 3668657
1 1 67092300 31558808
1 1 55090500 3463494
1 1 50080000 710481900
1 1 70070010 852616202
1 1 45070024 210550005
1 1 00 1501824
1 1 00 1501832
1 1 00 539290858
1 1 00 9290701
1 1 06 5073321010
1 1 06 94012341
1 1 10 12345008
1 1 10 87654008
1 1 17 0446786040
1 1 19 0200520016
1 1 19 0240334000
1 1 24 1306118605
1 1 24 0000138301
1 1 24 3307118608
1 1 24 9307118603
1 1 25 521382181
1 1 26 0005501024
1 1 26 0520309001
1 1 26 1111118111
1 1 27 2847169488
1 1 28 19999000
1 1 28 9130000201
1 1 29 3145863029
1 1 31 1000000524
1 1 31 1000000583
1 1 32 0121114867
1 1 32 0122116979
1 1 32 1709107983
1 1 32 9030101192
1 1 32 9141405
1 1 32 9245500460
1 1 33 48658
1 1 33 84956
1 1 34 9913000700
1 1 34 9914001000
1 1 35 0000101599
1 1 35 0000101709
1 1 35 0000102349
1 1 35 0000102921
1 1 35 0000107451
1 1 35 0000108443
1 1 35 101599
1 1 35 108443
1 1 36 113178
1 1 36 146666
1 1 37 624315
1 1 37 632500
1 1 38 1100660
1 1 38 191919
1 1 39 10019400
1 1 39 200205
1 1 40 1258345
1 1 40 3231963
1 1 41 0166805317
1 1 41 4013410024
1 1 41 4016660195
1 1 41 4019151002
1 1 41 4019310079
1 1 41 4019340829
1 1 42 59498
1 1 42 59510
1 1 43 6135244
1 1 43 9516893476
1 1 44 2618040504
1 1 44 889006
1 2 45 0000012340
1 2 45 0100114240
1 2 45 0994681254
1 2 45 1000199999
1 1 45 3545343232
1 1 45 4013410024
1 1 46 0235468612
1 1 46 0837890901
1 1 46 1041447600
1 1 47 1003554450
1 1 47 1018000
1 1 50 4000005001
1 1 50 4444442001
1 1 51 0000156071
1 1 51 0000156078
1 1 51 0001156071
1 1 51 0001156136
1 1 51 3199500501
1 1 54 4900010987
1 1 54 4964137395
1 1 56 290545005
1 1 56 9718304037
1 2 57 1909700805
1 2 57 5001050352
1 2 57 5045090090
1 1 57 7500021766
1 2 57 7777778800
1 1 57 7800028282
1 1 57 8100244186
1 1 57 9400001734
1 1 58 1015222224
1 1 58 1800293377
1 1 58 1800881120
1 1 58 3703169668
1 1 58 9200654108
1 1 61 0260760481
1 1 61 2063099200
1 1 62 5029076701
1 1 63 1234566
1 1 63 123456600
1 1 64 1206473010
1 1 64 5016511020
1 1 65 1234567400
1 1 65 1234567590
1 1 66 100150502
1 1 66 100154508
1 1 66 100154516
1 1 66 101154508
1 1 66 101154516
1 1 68 8889654328
1 1 68 987654324
1 1 68 987654328
1 1 69 1234567006
1 1 69 1234567900
1 1 69 9721134869
1 1 71 7101234007
1 1 73 3503398
1 1 73 7899100003
1 1 74 239314
1 1 74 239319
1 1 76 0006543200
1 1 76 123456
1 1 76 12345600
1 1 76 7876543100
1 1 76 9012345600
1 1 77 10338
1 1 77 1234554321
1 1 77 13844
1 1 77 65354
1 1 77 69258
1 1 78 7581499
1 1 78 9999999981
1 1 79 1550167850
1 1 79 3230012688
1 1 79 4230028872
1 1 79 5440001898
1 1 79 6330001063
1 1 79 7000149349
1 1 79 8000003577
1 1 79 9011200140
1 1 80 3199500501
1 1 80 340966
1 1 80 340968
1 1 81 0646440
1 1 81 1359100
1 1 81 3199500501
1 1 82 123897
1 1 82 3199500501
1 1 83 0000156071
1 1 83 0000156078
1 1 83 0001156071
1 1 83 0001156136
1 1 83 0099100002
1 1 84 100005
1 1 84 3199500501
1 1 84 393814
1 1 84 950360
1 1 85 0000156071
1 1 85 0000156078
1 1 85 0001156071
1 1 85 0001156136
1 1 85 3199100002
1 1 86 1001171
1 1 86 1009588
1 1 86 123897
1 1 86 3199500501
1 1 86 340968
1 1 87 0000000406
1 1 87 0000051768
1 1 87 0000100005
1 1 87 0000393814
1 1 87 0000950360
1 1 87 0010701590
1 1 87 0010720185
1 1 87 3199500501
1 1 88 1000500
1 1 88 2525259
1 1 88 90013000
1 1 88 92525253
1 1 88 99913003
1 1 89 1098506
1 1 89 218433000
1 1 89 32028008
1 1 90 0000156071
1 1 90 0000156073
1 1 90 0000156077
1 1 90 0000156132
1 1 90 0001156132
1 1 90 0001156136
1 1 90 0099100002
1 1 91 2974117000
1 1 91 2974118000
1 1 91 5281741000
1 1 91 5281770000
1 1 91 9952810000
1 1 91 9952812000
1 1 93 0000127783
1 1 93 0000127791
1 1 93 0000306754
1 1 93 0000671479
1 1 93 1277830000
1 1 93 1277910000
1 1 93 3067540000
1 1 93 6714790000
1 1 94 6782533003
1 1 95 0068007003
1 1 95 0847321750
1 1 95 6450060494
1 1 95 6454000003
1 1 96 0000000208
1 1 96 0000254100
1 2 96 0001300000
1 2 96 0099399999
1 1 96 0101115152
1 1 96 0301204301
1 1 96 9421000009
1 1 97 24010019
1 1 98 3009800016
1 1 98 5989800173
1 1 98 6719430018
1 1 98 9619319999
1 1 98 9619439213
1 1 98 9619509976
1 1 98 9619608118
1 1 99 0068007003
1 1 99 0847321750
1 1 a0 18761
1 1 a0 28290
1 1 a0 3287
1 1 a0 521003287
1 1 a0 54500
1 1 a1 0010030005
1 1 a1 0010030997
1 1 a1 1010030054
1 1 a21 3456789019
1 1 a21 5678901231
1 1 a21 6789012348
1 1 a22 3456789012
1 1 a31 1234567897
1 1 a31 0123456782
1 1 a32 9876543210
1 1 a32 1234567890
1 1 a32 0123456789
1 1 a41 0004711173
1 1 a41 0007093330
1 1 a42 0004711172
1 1 a42 0007093335
1 1 a43 1199503010
1 1 a43 8499421235
1 1 a44 9058440000
1 1 a44 0000905844
1 1 a44 5030101099
1 1 a45 8623420000
1 1 a45 0000862342
1 1 a51 9941510001
1 1 a51 9961230019
1 1 a51 9380027210
1 1 a51 9932290910
1 1 a52 0000251437
1 1 a52 0007948344
1 1 a52 0000159590
1 1 a52 0000051640
1 1 a61 800048548
1 1 a61 0855000014
1 1 a62 17
1 1 a62 55300030
1 1 a62 150178033
1 1 a62 600003555
1 1 a62 900291823
1 1 a71 19010008
1 1 a71 19010438
1 1 a72 19010660
1 1 a72 19010876
1 1 a72 209010892
1 1 a81 7436661
1 1 a81 7436670
1 1 a81 1359100
1 1 a82 7436660
1 1 a82 7436678
1 1 a82 0003503398
1 1 a82 0001340967
1 1 a83 0199100002
1 1 a83 0099100010
1 1 a83 2599100002
1 1 a84 0199100004
1 1 a84 2599100003
1 1 a84 3199204090
1 1 a91 5043608
1 1 a91 86725
1 1 a92 504360
1 1 a92 822035
1 1 a92 32577083
1 1 b0 1000000406
1 1 b0 1035791538
1 1 b0 1126939724
1 1 b0 1197423460
1 1 b11 1434253150
1 1 b11 2746315471
1 1 b12 7414398260
1 1 b12 8347251693
1 1 b21 0020012357
1 1 b21 0080012345
1 1 b21 0926801910
1 1 b21 1002345674
1 1 b22 8000990054
1 1 b22 9000481805
1 1 b31 1000000060
1 1 b31 0000000140
1 1 b31 0000000019
1 1 b31 1002798417
1 1 b31 8409915001
1 1 b32 9635000101
1 1 b32 9730200100
1 1 b41 9941510001
1 1 b41 9961230019
1 1 b41 9380027210
1 1 b41 9932290910
1 1 b42 0000251437
1 1 b42 0007948344
1 1 b42 0000051640
1 1 b51 0159006955
1 1 b51 2000123451
1 1 b51 1151043216
1 1 b51 9000939033
1 1 b52 0123456782
1 1 b52 0130098767
1 1 b52 1045000252 
1 1 B61 9110000000
1 1 B71 0700001529
1 1 B71 0730000019
1 1 B71 0001001008
1 1 B71 0001057887
1 1 B71 0001007222
1 1 B71 0810011825
1 1 B71 0800107653
1 1 B71 0005922372
1 1 B81 0734192657
1 1 B81 6932875274
1 1 B82 3145863029
1 1 B82 2938692523
1 1 B91 87920182
1 1 B91 87920187
1 1 B91 87920187
1 1 B91 41203755
1 1 B91 81069577
1 1 B91 61287958
1 1 B91 58467232
1 1 B92 7125633
1 1 B92 1253657
1 1 B92 4353631
1 1 C02 0082335729
1 1 C02 0734192657
1 1 C02 6932875274
1 1 C11 0446786040
1 1 C11 0478046940
1 1 C11 0701625830
1 1 C11 0701625840
1 1 C11 0882095630
1 1 C12 5432112349
1 1 C12 5543223456
1 1 C12 5654334563
1 1 C12 5765445670
1 1 C12 5876556788
1 1 C21 2394871426
1 1 C21 4218461950
1 1 C21 7352569148
1 1 C22 5127485166
1 1 C22 8738142564
1 1 c31 9294182
1 1 c31 4431276
1 1 c31 19919
1 1 c32 9000420530
1 1 c32 9000010006
1 1 c32 9000577650
1 1 c41 0000000019
1 1 c41 0000292932
1 1 c41 0000094455
1 1 c42 9000420530
1 1 c42 9000010006
1 1 c42 9000577650
1 -3 a1 0110030005
1 0 a1 0010030998
1 -3 a1 0000030005
1 0 a21 3456789012
1 0 a21 1234567890
1 0 a22 1234567890
1 0 a22 0123456789
1 0 a31 9876543210
1 0 a31 1234567890
1 0 a31 6543217890
1 0 a31 0543216789
1 0 a32 6543217890
1 0 a32 0543216789
1 0 a41 0004711172
1 0 a41 8623420004
1 0 a42 8623420000
1 0 a42 0001123458
1 0 a43 1299503117
1 0 a43 6099702031
1 0 a44 3994430000
1 0 a44 0000399443
1 0 a44 5533130000
1 0 a44 0000553313
1 0 a51 9941510002
1 0 a51 9961230020
1 0 a52 0000251438
1 0 a52 0007948345
1 0 a61 860000817
1 0 a61 810033652
1 0 a62 305888
1 0 a62 200071280
1 0 a71 19010660
1 0 a71 19010876
1 0 a71 209010892
1 0 a71 209010893
1 0 a72 209010893
1 0 a81 7436660
1 0 a81 7436678
1 0 a82 7436666
1 0 a82 7436677
1 0 a82 7436666
1 0 a82 7436677
1 0 a83 0199100004
1 0 a83 2599100003
1 0 a83 0099345678
1 0 a84 0099345678
1 0 a84 0099100110
1 0 a84 0199100040
1 0 a91 504360
1 0 a91 822035
1 0 a91 32577083
1 0 a91 86724
1 0 a92 86724
1 0 a92 292497
1 0 a92 30767208
1 0 b0 1000000405
1 0 b0 1035791539
1 -3 b0 8035791532
1 -3 b0 535791830
1 -3 b0 51234901
1 0 b11 7414398260
1 0 b11 8347251693
1 0 b11 0123456789
1 0 b11 2345678901
1 0 b11 5678901234
1 0 b12 0123456789
1 0 b12 2345678901
1 0 b12 5678901234
1 -3 b21 0020012399
1 0 b21 0080012347
1 -3 b21 0080012370
1 0 b21 0932100027
1 0 b21 3310123454
1 0 b22 8000990057
1 0 b22 8011000126
1 0 b22 9000481800
1 0 b22 9980480111
1 0 b31 0002799899
1 0 b31 1000000111
1 0 b32 9635100101
1 0 b32 9730300100
1 0 b41 9941510002
1 0 b41 9961230020
1 0 b42 0000251438
1 0 b42 0007948345
1 -3 b42 0000159590
1 0 b51 7414398260
1 0 b51 8347251693
1 0 b51 1151043211
1 0 b51 2345678901
1 0 b51 5678901234
1 0 b51 9000293707
1 0 b52 0159004165
1 0 b52 0023456787
1 0 b52 0056789018
1 0 b52 3045000333
1 0 B61 9111000000
1 0 B71 0001057886
1 0 B71 0003815570
1 0 B71 0005620516
1 0 B71 0740912243
1 0 B71 0893524479
1 0 B81 3145863029
1 0 B81 2938692523
1 0 B81 0132572975
1 0 B82 0132572975
1 0 B91 88034023
1 0 B91 43025432
1 0 B91 86521362
1 0 B91 61256523
1 0 B91 54352684
1 0 B92 2356412
1 0 B92 5435886
1 0 B92 9435414
1 0 C02 0132572975
1 0 C02 3038752371
1 0 C11 0446786240
1 0 C11 0478046340
1 0 C11 0701625730
1 0 C11 0701625440
1 0 C11 0882095130
1 0 C12 5432112341
1 0 C12 5543223458
1 0 C12 5654334565
1 0 C12 5765445672
1 0 C12 5876556780
1 0 C21 5127485166
1 0 C21 8738142564
1 0 C21 0328705282
1 0 C21 9024675131
1 0 C22 0328705282
1 0 C22 9024675131
1 0 c31 17002
1 0 c31 123451
1 0 c31 122448
1 0 c32 9000726558
1 0 c32 9001733457
1 0 c32 9000732000
1 0 c4a 0000000017
1 0 c4a 0000292933
1 0 c4a 0000094459
1 0 c4b 9000726558
1 0 c4b 9001733457
1 0 c4b 9000732000
1 1 c5 0000301168
1 1 c5 0000302554
1 1 c5 0300020050
1 1 c5 0300566000
1 1 c5 1000061378
1 1 c5 1000061412
1 1 c5 4450164064
1 1 c5 4863476104
1 1 c5 5000000028
1 1 c5 5000000391
1 1 c5 6450008149
1 1 c5 6800001016
1 1 c5 9000100012
1 1 c5 9000210017
1 1 c5 3060188103
1 1 c5 3070402023
1 2 c5 0030001230
1 2 c5 0040230000
1 2 c5 0050044000
1 2 c5 0059999999
1 2 c5 7000000000
1 2 c5 7023445000
1 2 c5 7000334560
1 2 c5 7099999999
1 2 c5 8500000000
1 2 c5 8502334450
1 2 c5 8599999999
1 1 c51 0000301168
1 1 c51 0000302554
1 1 c51 0300020050
1 1 c51 0300566000
1 1 c52 1000061378
1 1 c52 1000061412
1 1 c52 4450164064
1 1 c52 4863476104
1 1 c52 5000000028
1 1 c52 5000000391
1 1 c52 6450008149
1 1 c52 6800001016
1 1 c52 9000100012
1 1 c52 9000210017
1 1 c53 3060188103
1 1 c53 3070402023
1 1 c6 7000005024
1 1 c6 7008199027
1 1 c6 7002000023
1 1 c6 7000202027
1 0 c5 0000302589
1 0 c5 0000507336
1 0 c5 0302555000
1 0 c5 0302589000
1 0 c5 1000061457
1 0 c5 1000061498
1 0 c5 4864446015
1 0 c5 4865038012
1 0 c5 5000001028
1 0 c5 5000001075
1 0 c5 6450008150
1 0 c5 6542812818
1 0 c5 9000110012
1 0 c5 9000300310
1 0 c5 3081000783
1 0 c5 3081308871
1 0 c51 0000302589
1 0 c51 0000507336
1 0 c51 0302555000
1 0 c51 0302589000
1 0 c52 1000061457
1 0 c52 1000061498
1 0 c52 4864446015
1 0 c52 4865038012
1 0 c52 5000001028
1 0 c52 5000001075
1 0 c52 6450008150
1 0 c52 6542812818
1 0 c52 9000110012
1 0 c52 9000300310
1 0 c53 3081000783
1 0 c53 3081308871
1 0 c6 7000062022
1 0 c6 7006003027
1 0 c6 7003306026
1 0 c6 7001501029
2 2 10000 00243551900
2 2 10000 10412006400
2 1 14000 00115303384
2 1 14000 00159001040
2 1 31000 00000975409
2 1 40000 59990080003
2 1 40000 68136400006
2 -17 -12000 04975889900
2 -17 -60000 1740400
2 -17 -60000 7622670
2 -16 12000 04975889900
2 1 15000 611803412
2 -16 12000 04975889900
2 -4 20851 2100251301
2 1 31000 109313032
2 1 p001b1100039310003934800191e8bb45410 243551900
2 1 p001b1100039310003934800191e8bb45410 10511872300
2 1 p001b1100039310003934800191e8bb45410 60420313607
2 1 p001b1100039310003934800191e8bb45410 870149200
2 1 p001b1100039310003934800191e8bb45410 10412006400
2 1 p001b123 184800071
2 1 p001a1100019310 700320401
2 1 p001b121 115130889
2 1 p0019121 98000821896
2 1 pb439341b548421b239341001b131 18600
2 1 pb439341b548421b239341001b131 16500
2 1 pb439341b548421b239341001b131 16640
2 1 pb439341b548421b239341001b131 31994814
2 1 p001b1100039310003934800191e8bb45410 00405623703
2 1 p001b1100039310003934800191e8bb45410 00000200800
2 1 p001b1100039310003934800191e8bb45410 60013017000
2 1 p001b1100039310003934800191e8bb45410 66013446400
2 1 p001b1100039310003934800191e8bb45410 76413138500
2 1 p0018110 975409
2 1 pb957510bc57510001b110 59990080003
2 1 pb957510bc57510001b110 94111900000
2 1 pb957510bc57510001b110 68136400006
2 1 p0039348001b110 90027204700
2 1 pb957510001b121 155023007
2 1 pb957510001b121 95172101000
2 1 p001b166 7598656
2 1 p001b166 7000058
