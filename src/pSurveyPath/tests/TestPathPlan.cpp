#include "../PathPlan.h"
#include "catch.hpp"
#include "gnuplot-iostream.h"
#include <tuple>
//#include <stringstream>

template <class T>
std::string ListString(const T& print_list) {
  std::stringstream list_els;
  for (const auto& element : print_list) {
    list_els << element << " ";
  }
  return list_els.str();
}

std::string PrintPath(std::list<EPoint> print_list) {
  std::stringstream list_els;
  for (const auto& element : print_list) {
    list_els << "(" << element.transpose() << ") ";
  }
  return list_els.str();
}

std::pair<std::size_t, std::size_t> ProcessRemoveBend(std::list<EPoint>& path_bend,
  PathPlan planner, unsigned int plot_num, bool remove_intersects = true) {
  // Plot before
  Gnuplot gp;
  gp << "set terminal png size 1000,1000 enhanced font 'Verdana,10'\n";
  gp << "set output 'bend_graph" << plot_num << ".png'\n";
  gp << "set loadpath 'src/pSurveyPath/tests/'\n";
  gp << "load 'parula.pal'\n";
  gp << "load 'noborder.cfg'\n";
  //gp << "set style line 1 lt 1 lc rgb '#1B9E77'\n";
  //gp << "set style line 2 lt 1 lc rgb '#D95F02'\n";
  gp << "plot '-' with linespoints title 'Before' ls 11 lw 1 ps 1";
  gp << ", '-' with linespoints title 'After' ls 12 lw 1 ps 1\n";

  //For the raw examples, remove intersects first
  if (remove_intersects) {
    planner.RemoveIntersects(path_bend);
  }

  auto num_before = path_bend.size();
  gp.send1d(path_bend);
  planner.RemoveBends(path_bend);
  auto num_after = path_bend.size();

  //Plot after
  gp.send1d(path_bend);

  INFO("Points before: " << num_before << "\nPoints after: " << num_after)

  return std::make_pair(num_before, num_after);
}

TEST_CASE("Test Index selection") {
  std::list<std::size_t> test_list;
  unsigned int num_entries(10);
  for (unsigned int i = 0; i < num_entries; i++) {
    test_list.push_back(i);
  }

  REQUIRE(test_list.size() == num_entries);

  SECTION("Selecting middle indicies") {
    std::list<std::size_t> selection_ind = {3, 4, 5};
    PathPlan::SelectIndicies(test_list, selection_ind);
    INFO("Test List: " << ListString(test_list));
    REQUIRE(test_list.size() == 3);
    REQUIRE(test_list.front() == 3);
    REQUIRE(test_list.back() == 5);
  }

  SECTION("Selecting only first index") {
    std::list<std::size_t> selection_ind = {0};
    PathPlan::SelectIndicies(test_list, selection_ind);
    INFO("Test List: " << ListString(test_list));
    REQUIRE(test_list.size() == 1);
    REQUIRE(test_list.front() == 0);
    REQUIRE(test_list.back() == 0);
  }

  SECTION("Selecting only last index") {
    std::list<std::size_t> selection_ind = {num_entries - 1};
    PathPlan::SelectIndicies(test_list, selection_ind);
    INFO("Test List: " << ListString(test_list));
    REQUIRE(test_list.size() == 1);
    REQUIRE(test_list.front() == 9);
    REQUIRE(test_list.back() == 9);
  }
}

TEST_CASE("Intersection removal") {
  SECTION("Removal from the middle") {
    std::list<EPoint> path = {EPoint(0, 2), EPoint(3, 2), EPoint(5, 0),
      EPoint(4,0), EPoint(5,2), EPoint(7, 1)};
    INFO("Path before processing: \n" << PrintPath(path));

    PathPlan::RemoveIntersects(path);
    INFO("Path after processing: \n" << PrintPath(path));

    REQUIRE(path.size() == 4);
    REQUIRE(*std::next(path.begin(), 2) == EPoint(5, 2));
  }

  SECTION("Removal of multiple segs") {
    std::list<EPoint> path = {EPoint(0, 2), EPoint(2, 2), EPoint(4, 1),
      EPoint(5, 0), EPoint(3, 0), EPoint(4, 2), EPoint(6, 2), EPoint(8, 2)};
    INFO("Path before processing: \n" << PrintPath(path));

    PathPlan::RemoveIntersects(path);
    INFO("Path after processing: \n" << PrintPath(path));

    REQUIRE(path.size() == 5);
    REQUIRE(*std::next(path.begin(), 2) == EPoint(4, 2));
  }

  SECTION("First seg has intersect") {
    std::list<EPoint> path = {EPoint(0, 1), EPoint(2, 0), EPoint(1, 0),
      EPoint(2, 1), EPoint(3, 1)};
    INFO("Path before processing: \n" << PrintPath(path));

    PathPlan::RemoveIntersects(path);
    INFO("Path after processing: \n" << PrintPath(path));

    REQUIRE(path.size() == 3);
    REQUIRE(path.front() == EPoint(0, 1));
    REQUIRE(*std::next(path.begin(), 1) == EPoint(2, 1));
  }

  SECTION("Last seg has intersect") {
    std::list<EPoint> path = {EPoint(0, 1), EPoint(2, 1), EPoint(3, 1),
      EPoint(4, 0), EPoint(3, 0), EPoint(4, 1)};
    INFO("Path before processing: \n" << PrintPath(path));

    PathPlan::RemoveIntersects(path);
    INFO("Path after processing: \n" << PrintPath(path));

    REQUIRE(path.size() == 4);
    REQUIRE(path.back() == EPoint(4, 1));
    REQUIRE(*std::prev(path.end(), 2) == EPoint(3, 1));
  }
}

TEST_CASE("Test sharp bend removal") {
  //INFO("Before RecordSwath()");
  RecordSwath record_swath;
  //INFO("Before XYPolygon()");
  XYPolygon poly;
  //INFO("Before PathPlan()");
  PathPlan planner(record_swath, BoatSide::Port, poly);



  // gp << "splot ";

  SECTION("Test Path 1") {
    PathList path_bend = {EPoint(359698.73180335463, 4762590.5059377663), EPoint(359702.27830913838, 4762595.9642190039), EPoint(359704.95561734505, 4762600.5085234856), EPoint(359710.64734154526, 4762611.2275830591), EPoint(359717.58361402387, 4762622.092344759), EPoint(359722.71514325042, 4762631.5202486292), EPoint(359725.65741321596, 4762637.0815351959), EPoint(359732.73577035125, 4762649.4544684328), EPoint(359735.76651445765, 4762655.0013888543), EPoint(359742.11231337173, 4762665.5479164729), EPoint(359743.81787657645, 4762669.09914097), EPoint(359751.89029857889, 4762685.2470093463), EPoint(359756.11343865155, 4762691.4223660184), EPoint(359767.18961856433, 4762707.0848042835), EPoint(359770.20937080885, 4762710.0032711066), EPoint(359772.66829854809, 4762714.1327710291), EPoint(359778.57466792042, 4762724.7644340182), EPoint(359785.79751449887, 4762736.6594293211), EPoint(359789.02970252302, 4762741.9229452284), EPoint(359787.40984154062, 4762738.8175284229), EPoint(359789.5877853196, 4762746.6962104887), EPoint(359802.90446883679, 4762775.5350233773), EPoint(359831.35136983555, 4762766.8613436911), EPoint(359821.01896387507, 4762758.4582266957), EPoint(359825.07354731532, 4762799.503167266), EPoint(359831.43004730344, 4762809.2846533591), EPoint(359834.85872489924, 4762814.040661539), EPoint(359837.27433524252, 4762818.3826773455), EPoint(359844.31069087662, 4762830.3406309029), EPoint(359844.24100557412, 4762830.5393730924), EPoint(359850.59455097641, 4762847.2412675694), EPoint(359854.74367674981, 4762853.4675579639), EPoint(359866.96153852186, 4762875.7935591321), EPoint(359872.08605226805, 4762881.2947372533), EPoint(359870.66208591318, 4762878.4617782617), EPoint(359876.06046852627, 4762892.5595519012), EPoint(359877.94615593285, 4762895.2119141584), EPoint(359887.58689890231, 4762909.6363424426), EPoint(359889.14216015855, 4762909.0447817687), EPoint(359884.51357670588, 4762916.8781418307), EPoint(359885.195426722, 4762918.3226308674), EPoint(359885.28428429621, 4762939.0760829914), EPoint(359892.31046509778, 4762953.7577600526), EPoint(359895.54710570857, 4762962.052481316), EPoint(359905.91765860975, 4762981.6169737522), EPoint(359912.73693811876, 4762991.9147225171), EPoint(359922.43629955227, 4763006.6703013545), EPoint(359924.05725564586, 4763008.8811783101), EPoint(359934.07919936825, 4763021.9839935191), EPoint(359988.22120255907, 4763040.7124714246), EPoint(360000.78226290049, 4763006.5258415062), EPoint(359966.83099161542, 4763038.0892774677), EPoint(359948.63131866592, 4763038.2216126071), EPoint(359948.35300107603, 4763040.5247643897), EPoint(359953.98644372344, 4763060.2649731394), EPoint(359958.30050064338, 4763069.9776490461), EPoint(359968.47226999549, 4763086.4594381452), EPoint(359974.62822832208, 4763096.4834942231), EPoint(359981.72480504774, 4763107.1430340949), EPoint(359987.42019512842, 4763114.5521491775), EPoint(359989.7735491824, 4763118.429057166), EPoint(359990.39809679979, 4763120.891922106), EPoint(359997.17625900218, 4763134.5744723761), EPoint(360000.94299859356, 4763141.8823496532), EPoint(360008.81221308332, 4763153.8457104554), EPoint(360012.48533436225, 4763160.466109952), EPoint(360021.70127619558, 4763177.4088526824), EPoint(360031.48855158954, 4763188.0270230882), EPoint(360032.76777829399, 4763189.6582871871), EPoint(360035.32355632662, 4763195.5525668757), EPoint(360039.78106204973, 4763201.0358408578), EPoint(360038.83558226208, 4763199.3385219546), EPoint(360049.44708558038, 4763226.1034816597), EPoint(360080.21741819795, 4763250.2305333409), EPoint(360077.73191092809, 4763249.4622250404), EPoint(360064.22009198059, 4763251.6043260591), EPoint(360071.40947133262, 4763263.4580341782), EPoint(360072.55085720657, 4763264.7961321017), EPoint(360076.5383720622, 4763274.015740796), EPoint(360081.38622047059, 4763284.6338322675), EPoint(360083.11305763019, 4763287.3333489737), EPoint(360090.35091346723, 4763303.5687761661), EPoint(360096.43453033728, 4763313.9177138256), EPoint(360101.03846765211, 4763322.6949816402), EPoint(360105.54653065227, 4763331.5689526405), EPoint(360113.72260631231, 4763344.357760042), EPoint(360117.37077925541, 4763350.4706811486), EPoint(360124.17538763332, 4763362.014269406), EPoint(360134.26639648154, 4763376.4334316989), EPoint(360138.36146866425, 4763382.1691250568), EPoint(360140.69921795127, 4763385.6769898888)};
    // path_bend1 = path_bend{68:82};
    // path_bend2 = path_bend{44:60};

    auto before_after = ProcessRemoveBend(path_bend, planner, 1);

    REQUIRE(before_after.second == 76);
  }

  SECTION("Test Path 2") {
    PathList long_elim1 = {EPoint(359582.12410424626, 4762667.292814943), EPoint(359587.92312658968, 4762671.1564266328), EPoint(359609.96669203276, 4762680.6489642635), EPoint(359616.5566125412, 4762678.6956708357), EPoint(359607.72074091423, 4762671.888678981), EPoint(359609.63341794425, 4762675.5841281917), EPoint(359625.02917290881, 4762690.5097677298), EPoint(359632.0580726745, 4762696.6116010118), EPoint(359638.84945857571, 4762701.057564022), EPoint(359642.11328747025, 4762705.109729588), EPoint(359643.75949175423, 4762707.1764429044), EPoint(359644.0938559015, 4762707.4122816753), EPoint(359644.09745468805, 4762708.9783482915), EPoint(359645.18978093559, 4762722.0829701442), EPoint(359651.45994976722, 4762736.5690601114), EPoint(359658.63910900662, 4762749.5429683942), EPoint(359658.83392982726, 4762750.4726691786), EPoint(359665.37297805981, 4762764.7899404587), EPoint(359666.15934641531, 4762768.2465715613), EPoint(359671.08021355671, 4762779.2403904675), EPoint(359670.99959680805, 4762780.3526290143), EPoint(359680.15344226034, 4762806.4437977299), EPoint(359686.11320961168, 4762817.2620036509), EPoint(359689.01835457771, 4762823.0145072853), EPoint(359695.13533467072, 4762835.7292982209), EPoint(359700.92261461006, 4762845.8030086104), EPoint(359711.91544702224, 4762861.2340816213), EPoint(359714.59137908131, 4762863.8340597078), EPoint(359716.75682353764, 4762868.4352983423), EPoint(359719.07632045436, 4762872.0940804426), EPoint(359722.43461212341, 4762882.0578046916), EPoint(359733.05409707734, 4762901.3265012205), EPoint(359736.76319103822, 4762907.8301631119), EPoint(359738.31985084451, 4762910.737685659), EPoint(359755.61943995312, 4762884.484686642), EPoint(359756.58459357289, 4762881.4116219589), EPoint(359749.00000412838, 4762942.9128772505), EPoint(359791.56091791176, 4762971.4737816788), EPoint(359783.98262714536, 4762903.4016061043), EPoint(359753.22915462602, 4762942.713538358), EPoint(359752.81223214616, 4762955.4710798571), EPoint(359752.79828445078, 4762961.0242882613), EPoint(359760.61016945128, 4762998.917718607), EPoint(359765.56374095555, 4763011.0064783534), EPoint(359773.65762667509, 4763025.7749259463), EPoint(359776.35808339942, 4763029.4325127862), EPoint(359782.79975650966, 4763042.0453094942), EPoint(359784.05160806217, 4763043.0885546086), EPoint(359785.27430011233, 4763057.0369646493), EPoint(359795.37391907413, 4763074.3103535343), EPoint(359811.72125267686, 4763094.6523889638), EPoint(359817.90608026012, 4763100.7381948391), EPoint(359819.0315862234, 4763102.349839353), EPoint(359842.83181778545, 4763124.4220846631), EPoint(359855.45651211316, 4763133.655241671), EPoint(359904.92951125628, 4763127.0491673779), EPoint(359847.75061628921, 4763082.7356207874), EPoint(359847.93016643089, 4763128.4086687509), EPoint(359853.20565971063, 4763136.5465494199), EPoint(359864.51300091232, 4763159.6853850838), EPoint(359872.25640000607, 4763136.1802101545), EPoint(359874.58488020563, 4763131.0543675479), EPoint(359867.98069281329, 4763163.0995499622), EPoint(359875.08422010689, 4763180.7935814876), EPoint(359883.02330823767, 4763198.3842862779), EPoint(359889.73934293998, 4763207.5757210813), EPoint(359890.9494909012, 4763208.9778236458), EPoint(359894.21797123272, 4763217.0495575164), EPoint(359897.78988667001, 4763223.607979198), EPoint(359900.3192444173, 4763234.1398315243), EPoint(359910.23110444471, 4763253.9029717054), EPoint(359913.47611918353, 4763258.1500553247), EPoint(359914.44639327179, 4763257.9596775742), EPoint(359913.4879479013, 4763262.4823830016), EPoint(359919.08883218648, 4763279.3537203968), EPoint(359930.01543693076, 4763299.2659517694), EPoint(359937.64490565943, 4763310.0034863343), EPoint(359939.36699538404, 4763311.9012246523), EPoint(359942.67921192583, 4763319.2630034266), EPoint(359944.98728203098, 4763325.2694142498), EPoint(359951.00678033341, 4763336.577849336), EPoint(359961.61840227951, 4763353.3734683171), EPoint(359967.52811064216, 4763363.3260004343), EPoint(359980.72627137642, 4763378.9454761874), EPoint(359987.37590637687, 4763384.6683263676), EPoint(359990.42058087699, 4763387.7758319294), EPoint(359999.37622193986, 4763395.9226328293), EPoint(360006.40798739623, 4763400.9262128836), EPoint(360013.0790663145, 4763409.0251081046), EPoint(360016.11200576928, 4763414.4042408392), EPoint(360016.18061808281, 4763414.6203761175), EPoint(360026.33625578188, 4763430.6171192462), EPoint(360026.68096942396, 4763431.5342841381)};
    // long_elim1 = long_elim1{68:92};
    auto before_after = ProcessRemoveBend(long_elim1, planner, 2);

    REQUIRE(before_after.second == 79);
  }

  SECTION("Test Path 3") {
    PathList long_elim2 = {EPoint(359857.67122730939, 4763499.2868956393), EPoint(359856.85607340693, 4763487.8126620399), EPoint(359878.26646413654, 4763502.5361341117), EPoint(359881.05951185996, 4763507.4117382383), EPoint(359857.69000284962, 4763490.9659897638), EPoint(359849.07590692054, 4763481.8792166635), EPoint(359848.59396758839, 4763480.7860900797), EPoint(359836.67128423491, 4763471.7518834397), EPoint(359823.09664255579, 4763451.2093687756), EPoint(359823.19982432073, 4763450.6436365712), EPoint(359809.28295792983, 4763437.0775498264), EPoint(359806.69124907069, 4763431.8274683598), EPoint(359797.66191247461, 4763414.2191108484), EPoint(359796.92633249081, 4763412.6820278987), EPoint(359791.52326811961, 4763404.0638983836), EPoint(359785.8383383489, 4763395.2152651269), EPoint(359778.05591619987, 4763381.8891693912), EPoint(359767.7568965801, 4763361.5742648365), EPoint(359780.66386741202, 4763369.9388347473), EPoint(359782.73334573233, 4763372.706382771), EPoint(359755.39073397196, 4763345.9962168708), EPoint(359750.97491434764, 4763338.5183835337), EPoint(359745.45851635013, 4763327.5876527168), EPoint(359739.78587316815, 4763309.5687169842), EPoint(359740.58966896206, 4763303.7582004424), EPoint(359736.57827852719, 4763295.981372443), EPoint(359731.68893682334, 4763284.2375732129), EPoint(359730.03490222694, 4763282.4989343798), EPoint(359725.69936505012, 4763278.8307614112), EPoint(359717.48182418389, 4763267.3144949879), EPoint(359706.09221610677, 4763237.8456644174), EPoint(359707.5607307569, 4763215.6592722991), EPoint(359708.16326363431, 4763218.3572666338), EPoint(359704.68135185062, 4763224.1110412646), EPoint(359704.27509084827, 4763222.9963739635), EPoint(359699.22595258179, 4763216.1396284532), EPoint(359687.65172220324, 4763202.612646509), EPoint(359680.6607694485, 4763190.5892739594), EPoint(359679.72345400171, 4763181.60749109), EPoint(359675.88042975799, 4763180.1565526044), EPoint(359668.96202951885, 4763171.4836872267), EPoint(359665.35603954893, 4763168.3080262523), EPoint(359653.76065962511, 4763155.0739680687), EPoint(359646.25308436569, 4763144.047146379), EPoint(359644.41075775551, 4763141.6804520758), EPoint(359633.38293226296, 4763124.8932237858), EPoint(359628.93777717563, 4763120.4540667711), EPoint(359622.74454151827, 4763110.2013765359), EPoint(359613.72361065674, 4763095.0226790328), EPoint(359609.16743742087, 4763084.4031274058), EPoint(359607.5212388548, 4763079.9194406671), EPoint(359602.28781824588, 4763066.2533277059), EPoint(359598.75541124371, 4763056.9769601924), EPoint(359596.43151059683, 4763051.6583312694), EPoint(359588.54631800839, 4763033.1722384635), EPoint(359587.19715484302, 4763029.3199132392), EPoint(359582.98818027461, 4763016.116079798), EPoint(359578.65266459947, 4762997.2590709338), EPoint(359578.50394429016, 4762988.364710534), EPoint(359576.57197468943, 4762984.0965202162), EPoint(359576.38976585626, 4762952.7323705601), EPoint(359577.61944702436, 4762945.2144404128), EPoint(359578.28873843001, 4762944.548083649), EPoint(359598.02851704624, 4762952.6663959511), EPoint(359598.15311147878, 4762959.805674769), EPoint(359567.04998563201, 4762934.5895469114), EPoint(359563.19557050575, 4762928.3813784365), EPoint(359557.66117777425, 4762914.6394199803), EPoint(359555.84377002297, 4762908.4907225696), EPoint(359554.47455316165, 4762903.0956070526), EPoint(359545.91938519262, 4762897.9163096473), EPoint(359534.54457379878, 4762877.8280922445), EPoint(359531.17094537051, 4762868.5773500549), EPoint(359525.55289783678, 4762858.7207524516), EPoint(359520.69602890022, 4762841.0785324713), EPoint(359517.92335095722, 4762834.481349173), EPoint(359513.82003837964, 4762822.2184557645), EPoint(359513.44921427255, 4762808.6518442165), EPoint(359531.87130511121, 4762810.4430109002), EPoint(359534.18869178562, 4762818.9253123179), EPoint(359513.44460092962, 4762772.8914689347), EPoint(359556.19366760645, 4762804.9907105155), EPoint(359508.23975882621, 4762805.95431339), EPoint(359517.52081512334, 4762792.6543500545), EPoint(359522.37785715837, 4762796.7019845461), EPoint(359496.42134769401, 4762798.8868875038), EPoint(359494.39038095559, 4762800.8987096408), EPoint(359477.10391312849, 4762799.714700548), EPoint(359462.04392811179, 4762795.3337785723), EPoint(359457.11051564728, 4762794.4573274152), EPoint(359440.53564153996, 4762786.4994551511), EPoint(359432.75181373878, 4762783.9983078577), EPoint(359423.74285936216, 4762783.5886100689), EPoint(359413.26253919047, 4762785.2884075763), EPoint(359398.48188116064, 4762785.5619460652), EPoint(359374.56693884428, 4762777.6152879391), EPoint(359367.66117885913, 4762773.5450909566), EPoint(359364.03989688063, 4762771.6055361582)};
    // long_elim2 = long_elim2{58:97};
    auto before_after = ProcessRemoveBend(long_elim2, planner, 3);

    REQUIRE(before_after.second == 79);
  }

  SECTION("Test Path 4") {
    PathList elim_end1 = {EPoint(359980.15779347485, 4763459.8809305364), EPoint(359972.89259364147, 4763448.0761056505), EPoint(359973.56690819567, 4763448.3827140238), EPoint(359963.25811429659, 4763413.7071129968), EPoint(359963.82657529472, 4763413.3915569382), EPoint(359965.70629783167, 4763442.8396715103), EPoint(359959.34286859335, 4763440.6799630169), EPoint(359937.77653963526, 4763422.6934370901), EPoint(359925.1376235346, 4763410.3712384515), EPoint(359910.5037613989, 4763388.0878775408), EPoint(359907.74857952708, 4763384.1027459186), EPoint(359904.86981753935, 4763380.1336542657), EPoint(359892.98244860419, 4763362.8431354212), EPoint(359885.44506636658, 4763345.5393416146), EPoint(359893.15652008721, 4763352.2080130158), EPoint(359896.45008158474, 4763354.4621559102), EPoint(359881.38786966662, 4763339.2976908153), EPoint(359865.82458323584, 4763311.7060450464), EPoint(359861.24533608265, 4763291.5952575188), EPoint(359861.1741458152, 4763290.75214699), EPoint(359858.69531712565, 4763285.244876246), EPoint(359856.32383182342, 4763282.8699452011), EPoint(359853.59835394431, 4763280.627030394), EPoint(359844.81670186849, 4763271.6049015801), EPoint(359833.31383851619, 4763249.5377598852), EPoint(359829.79526264762, 4763241.1308765803), EPoint(359825.07168009278, 4763233.7479471778), EPoint(359819.99419462384, 4763222.3706840025), EPoint(359815.81499067566, 4763211.6510080416), EPoint(359810.58023468562, 4763196.3535681255), EPoint(359813.15967693413, 4763195.5783324288), EPoint(359813.3114218493, 4763196.7087037312), EPoint(359805.68838628603, 4763183.7798186559), EPoint(359803.06225802371, 4763178.7005322743), EPoint(359805.7185165452, 4763178.5065955659), EPoint(359806.12753803504, 4763179.7342993831), EPoint(359786.85544087173, 4763159.6528691752), EPoint(359777.64503777167, 4763147.3660809956), EPoint(359774.79475256504, 4763144.3742191242), EPoint(359764.56036438548, 4763132.8751701396), EPoint(359759.04154718894, 4763124.6197667774), EPoint(359754.02341379318, 4763117.6862093033), EPoint(359740.44355899241, 4763096.1475020815), EPoint(359735.6780327828, 4763091.5534554468), EPoint(359727.06975713023, 4763066.1986309495), EPoint(359727.60402790812, 4763067.1487698955), EPoint(359723.78682200535, 4763061.7723367848), EPoint(359722.89208437921, 4763059.9852161026), EPoint(359716.52575819718, 4763050.0105659897), EPoint(359706.80686692597, 4763028.2861245116), EPoint(359704.30059389322, 4763021.2056996254), EPoint(359699.37516914209, 4763008.1474650148), EPoint(359696.08368075645, 4762994.7910188874), EPoint(359693.55408648524, 4762987.7470995728), EPoint(359690.16724816751, 4762974.3429044196), EPoint(359687.14121402596, 4762958.5033114646), EPoint(359685.853015356, 4762953.7636592761), EPoint(359683.2627165718, 4762942.0014257794), EPoint(359683.38577142282, 4762934.0472324807), EPoint(359682.73928546812, 4762932.236544298), EPoint(359679.7604120489, 4762925.790771774), EPoint(359678.04115148284, 4762922.5152604552), EPoint(359667.34051152307, 4762897.9572617961), EPoint(359666.89648540487, 4762892.6522202287), EPoint(359666.82108699786, 4762892.5104514649), EPoint(359667.90730609151, 4762894.9711005464), EPoint(359658.44700932829, 4762886.8053592965), EPoint(359648.81131393334, 4762873.7120465608), EPoint(359636.88371798524, 4762851.7073287833), EPoint(359634.96591196104, 4762844.7573926914), EPoint(359629.75231980562, 4762833.8556333771), EPoint(359625.0349513322, 4762816.7668533158), EPoint(359624.28134310269, 4762813.8783769459), EPoint(359642.07965243247, 4762803.0743191708), EPoint(359643.88340642664, 4762808.7550913468), EPoint(359615.56863671757, 4762793.2612905959), EPoint(359610.93212508451, 4762782.7066502701), EPoint(359604.6217243335, 4762767.0223552035), EPoint(359599.5499873744, 4762755.0300722886), EPoint(359595.9838953939, 4762746.1209989153), EPoint(359590.24810804985, 4762720.0520195551), EPoint(359610.05609154073, 4762691.0007341327), EPoint(359629.21258338547, 4762740.8356882706), EPoint(359595.36486394925, 4762736.3718347726), EPoint(359586.08647689532, 4762728.4546481147), EPoint(359586.1271319646, 4762727.5761762541), EPoint(359589.10253610945, 4762729.0251211422), EPoint(359576.25701625191, 4762725.9375959272), EPoint(359571.03137922462, 4762726.7504354063), EPoint(359550.20468692534, 4762723.9131927453), EPoint(359536.44194186199, 4762719.203858505), EPoint(359524.85776215896, 4762712.5769497668), EPoint(359516.64135219128, 4762707.9494475676), EPoint(359508.63782349904, 4762702.8416332379)};
    // elim_end1 = elim_end1{3:};
    auto before_after = ProcessRemoveBend(elim_end1, planner, 4);

    REQUIRE(before_after.second == 72);
  }

  SECTION("Test Path 5") {
    PathList elim_end2 = {EPoint(359700.5645612941, 4763569.9049124932), EPoint(359696.56682828901, 4763608.7537348066), EPoint(359667.30366875918, 4763594.208486761), EPoint(359662.73382248828, 4763588.3811976658), EPoint(359651.64224280627, 4763567.7940597804), EPoint(359651.08793177729, 4763565.0579684302), EPoint(359648.27349018055, 4763559.4712883364), EPoint(359642.7301511151, 4763551.4729395006), EPoint(359636.64736856834, 4763539.6293341052), EPoint(359629.18354503094, 4763527.4993750975), EPoint(359627.06211791275, 4763520.7591126328), EPoint(359625.26652493596, 4763519.0160296485), EPoint(359617.96382132987, 4763505.3826277712), EPoint(359612.81757334206, 4763492.7722802181), EPoint(359608.97072381148, 4763481.015237011), EPoint(359602.57667178544, 4763464.711222793), EPoint(359602.71433072037, 4763464.5964088822), EPoint(359599.66323812748, 4763462.7631950062), EPoint(359592.76051902684, 4763453.876866173), EPoint(359584.30855705199, 4763439.8581740307), EPoint(359578.97948543343, 4763423.981261258), EPoint(359574.45142146962, 4763413.3663159199), EPoint(359575.12087735836, 4763409.5486989925), EPoint(359570.24619531882, 4763405.2119030105), EPoint(359565.27439878264, 4763395.3134425767), EPoint(359557.84957093315, 4763378.3427133802), EPoint(359553.29728069698, 4763369.9548857771), EPoint(359550.07460785052, 4763360.4116979772), EPoint(359544.71378343203, 4763348.7131718164), EPoint(359541.65243280283, 4763341.506569827), EPoint(359539.04999787512, 4763337.3956270367), EPoint(359529.73759556364, 4763322.612287797), EPoint(359529.89968216675, 4763316.8799651582), EPoint(359526.13158196112, 4763312.9324019635), EPoint(359523.56915015035, 4763308.7907090522), EPoint(359517.13767492556, 4763294.9835041268), EPoint(359514.32334425469, 4763292.3563579004), EPoint(359508.05213916965, 4763284.3898897711), EPoint(359499.52454170201, 4763274.767287815), EPoint(359494.98062895116, 4763269.8038610267), EPoint(359484.52350941586, 4763247.2786864676), EPoint(359480.05692500365, 4763237.4429747798), EPoint(359478.93015129614, 4763236.2079887791), EPoint(359474.99768122035, 4763232.8152051689), EPoint(359469.6562927948, 4763228.5923477327), EPoint(359464.67222505278, 4763223.7896896321), EPoint(359460.09110311803, 4763220.047665339), EPoint(359445.141233859, 4763202.8095522597), EPoint(359436.85708428419, 4763191.9165711338), EPoint(359427.95328789367, 4763173.3887685938), EPoint(359424.26541675912, 4763164.7568100598), EPoint(359419.89670587389, 4763152.5729250303), EPoint(359419.38374927762, 4763146.9736061879), EPoint(359416.79462783848, 4763142.7842519656), EPoint(359407.84800232772, 4763116.4763735719), EPoint(359406.45787130634, 4763103.0474821534), EPoint(359405.87776550278, 4763087.9325147755), EPoint(359407.1301464478, 4763078.9913778221), EPoint(359407.51588199957, 4763065.7744216695), EPoint(359410.23062576645, 4763060.4989416916), EPoint(359409.81474900269, 4763038.1384183448), EPoint(359410.20912824781, 4763035.2698777858), EPoint(359417.09167788608, 4763028.4049715763), EPoint(359416.43530893343, 4763031.1709997747), EPoint(359409.9513340494, 4763006.9121865919), EPoint(359418.63713654794, 4762989.2298077857), EPoint(359414.67717585451, 4762999.336692811), EPoint(359414.70209405839, 4763015.7517858222), EPoint(359403.06913863984, 4763011.4768739846), EPoint(359391.91401961917, 4763003.6581120389), EPoint(359372.01374777005, 4762984.5877992678), EPoint(359363.71447833802, 4762970.9473257065), EPoint(359356.70681031316, 4762954.7326849271), EPoint(359353.65184228681, 4762943.3651411068), EPoint(359350.48468827771, 4762928.945585046), EPoint(359350.58874414588, 4762926.8903593374), EPoint(359347.11395823921, 4762916.0647208542), EPoint(359353.68769761221, 4762883.4213763801), EPoint(359382.16440037091, 4762914.195664146), EPoint(359366.81663756463, 4762928.4356595706), EPoint(359351.74435267766, 4762929.0885682506), EPoint(359327.04156216205, 4762922.5502518257), EPoint(359313.70494472928, 4762917.4058157569), EPoint(359312.0493690719, 4762914.6196836215), EPoint(359303.48660402629, 4762913.1357576074), EPoint(359295.0782309866, 4762909.6232288405), EPoint(359297.22773687204, 4762909.599320814), EPoint(359288.04203971423, 4762916.4196493831), EPoint(359270.36223583826, 4762913.1649981532), EPoint(359259.51576747833, 4762910.3531963313), EPoint(359252.14670451795, 4762911.1186173866), EPoint(359237.53813528514, 4762910.0215766281), EPoint(359237.95163681806, 4762908.6384116393), EPoint(359226.86607904593, 4762911.6507603331), EPoint(359208.40789930162, 4762910.8419081969), EPoint(359200.95804980211, 4762912.1137536699), EPoint(359184.23023293185, 4762907.3858547239), EPoint(359178.15712960094, 4762910.1404810483), EPoint(359165.45523626049, 4762912.8573901663), EPoint(359148.81868891144, 4762911.608959524), EPoint(359137.07227402588, 4762905.9670411553)};
    auto before_after = ProcessRemoveBend(elim_end2, planner, 5);

    REQUIRE(before_after.second == 78);
  }

  SECTION("Test Path 6") {
    PathList begin1 = {EPoint(359199.31274530472, 4762946.2258861894), EPoint(359201.20467422652, 4762946.4012990352), EPoint(359208.36326765327, 4762946.453385571), EPoint(359232.42781790608, 4762947.836972151), EPoint(359234.71488368034, 4762947.0128680132), EPoint(359248.0100861463, 4762945.9051908776), EPoint(359249.5278209034, 4762940.85004084), EPoint(359247.42728791386, 4762939.0210957089), EPoint(359268.13981250121, 4762948.1633667713), EPoint(359277.33251810563, 4762952.0586164948), EPoint(359296.14475949208, 4762955.1469698688), EPoint(359318.33935674297, 4762940.5036110776), EPoint(359305.32675520563, 4762938.2397169583), EPoint(359299.40507404343, 4762955.5903681396), EPoint(359314.6627559053, 4762963.145727423), EPoint(359343.12328688911, 4762963.6420175359), EPoint(359325.74050476804, 4762918.2942198757), EPoint(359310.30405884981, 4762942.9745851336), EPoint(359311.03185552551, 4762954.959876759), EPoint(359314.15217846452, 4762971.2306523295), EPoint(359319.28648617782, 4762987.016971116), EPoint(359330.54067931784, 4763006.0584214991), EPoint(359332.16267075087, 4763008.4982931819), EPoint(359352.8837522472, 4763029.9194030557), EPoint(359360.43682761508, 4763034.5145567628), EPoint(359376.41630704334, 4763043.8517471757), EPoint(359370.03308417357, 4763034.4318206552), EPoint(359367.05810704379, 4763033.6930076079), EPoint(359371.29000769398, 4763043.9858715497), EPoint(359370.89180566388, 4763048.9135741154), EPoint(359374.82619855925, 4763058.9633039227), EPoint(359374.47493772558, 4763056.0872165998), EPoint(359371.96361108084, 4763065.2012208337), EPoint(359369.14837763296, 4763073.7633798048), EPoint(359364.30532656616, 4763086.3908754578), EPoint(359362.44581653801, 4763098.5114856893), EPoint(359361.10788951855, 4763111.8738377802), EPoint(359360.53053993016, 4763131.292864372), EPoint(359367.69442075747, 4763157.01440892), EPoint(359368.24175924395, 4763157.7108529694), EPoint(359374.03411247069, 4763175.576478661), EPoint(359378.18437084806, 4763182.3913163869), EPoint(359387.54818489548, 4763197.3790640263), EPoint(359395.28356940276, 4763206.1897040159), EPoint(359402.12214115629, 4763216.8414839301), EPoint(359410.21152845578, 4763228.6307859942), EPoint(359419.76648382604, 4763239.2486702744), EPoint(359424.74600098073, 4763243.3285089387), EPoint(359439.26573907339, 4763254.0682981433), EPoint(359450.19544719387, 4763262.9503616439), EPoint(359449.18706312729, 4763260.862664626), EPoint(359444.29592223384, 4763255.1844911808), EPoint(359453.74297378666, 4763274.8334670886), EPoint(359457.03124556941, 4763280.1557821212), EPoint(359471.77412310767, 4763297.7262834357), EPoint(359475.65966936317, 4763301.9056596784), EPoint(359477.81065201247, 4763305.0684395134), EPoint(359481.53600205219, 4763315.4019757751), EPoint(359486.09710053558, 4763323.4980921596), EPoint(359494.62192642788, 4763336.7285288963), EPoint(359496.4700170657, 4763343.4265157543), EPoint(359501.96623805683, 4763349.8226850843), EPoint(359501.32839555317, 4763352.4684330691), EPoint(359505.60862888442, 4763363.1947145415), EPoint(359509.19707562996, 4763371.0106128128), EPoint(359517.42470366071, 4763390.2188974395), EPoint(359526.7853721393, 4763401.2755774101), EPoint(359526.8180965808, 4763399.8612825936), EPoint(359530.84936407668, 4763415.5689901914), EPoint(359542.38372931036, 4763414.0834351722), EPoint(359542.01251301239, 4763412.6667736452), EPoint(359544.50540353765, 4763437.4616250396), EPoint(359548.67319264024, 4763444.7780022416), EPoint(359558.18740745855, 4763458.9883602085), EPoint(359563.4003207963, 4763466.3804016113), EPoint(359569.04061995569, 4763475.1931067025), EPoint(359571.67027452041, 4763479.3987078145), EPoint(359573.62664887356, 4763484.7164659416), EPoint(359577.22896788374, 4763495.0404101089), EPoint(359577.13285658346, 4763499.7381285569), EPoint(359583.26985654928, 4763513.6011396339), EPoint(359587.00983571314, 4763524.1349840909), EPoint(359595.11954001198, 4763537.1091161938), EPoint(359598.13098734355, 4763546.0850273008), EPoint(359598.25496409001, 4763551.6247999519), EPoint(359604.25515914639, 4763563.0230399845), EPoint(359608.16621253302, 4763574.1370884068), EPoint(359611.47676829767, 4763580.00879029), EPoint(359617.63800738379, 4763596.6339707822), EPoint(359630.03876627603, 4763608.3117132494), EPoint(359634.81326289452, 4763613.7312221145), EPoint(359634.64791711041, 4763617.9473176096), EPoint(359640.23078692239, 4763628.5421363404)};
    // begin1 = begin1{2:};
    auto before_after = ProcessRemoveBend(begin1, planner, 6);

    REQUIRE(before_after.second == 72);
  }

  // SECTION("Bend At Beginning") {
  //   PathList begin = {EPoint()}
  // }
}
