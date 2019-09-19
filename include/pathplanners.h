#ifndef PLANNER_H
#define PLANNER_H
#include "aprilvideointerface.h"
#include <vector>
#include <utility>
#include <stack>
#include <cstring>

//the cell empty criteria is an adjustable parameter as well
class PathPlannerGrid{
  public:
    //the ids below are the indexes in the detections vector, not the actual tag ids
    static cv::RNG rng;
    cv::Scalar path_color;
    int goal_id;
    int robot_id;
    int origin_id;
    //the real tag id
    int robot_tag_id;
    //note the different use of r,c and x,y in the context of matrix and image respectively
    int cell_size_x;//cell size in pixels
    int cell_size_y;
    int threshold_value;
    std::vector<pt> path_points;
    std::vector<pair<int,int> > pixel_path_points;
    int total_points;
    int start_grid_x,start_grid_y;
    int goal_grid_x, goal_grid_y;
    //when sharing a map, make sure to share the below two values explicitly in your code, as sharing only the reference to map is not enough
    int rcells, ccells;
    std::vector<std::vector<nd> > &world_grid;//grid size is assumed to be manueveurable by the robot
   //std::vector<std::vector<nd> > world_grid;//not needed as a reference variable in case of imposing communication range constraints
    //the following matrix is used to encode local preference based on current place and parent place, one is added to avoid negative array index
    std::pair<int,int> aj[3][3][4];
    stack<pair<int,int> > sk;//stack is needed to remember all the previous points visited and backtrack, should be unique for every instance, used primarily by the incremental bsa
    
    //below two/three are used exclusively for incrementalbsa function, don't use for any other purpose
    int first_call;
    vector<bt> bt_destinations;
    int status; // 0- spiral; 1- return; 2- inactive, used in BSA type algorithms

//below variable is used collision avoidance algo:
    
    int last_grid_x, last_grid_y; //its the cordinates of the last cell bot was present in
    int next_target_index; //its the index of next target in path points
    int deadlock_check_counter; //if the value of this counter gets bigger then 1, it means there's deadlock
    std::pair<int, int> target_grid_cell;
    int deadlock_replanned; //stores the number of times the bot has replanned it's path in case of deadlock

//below variable is used for the bactrack selection algorithm as given in two way proximity

    std::pair <int, int> blockedcellcheck[3][3][2][3];

//below variable is used to store the unvisited empty neighbours by robots. They are same as bt points in the BSA-Cm bactrack scheme, but not so in case of other schemes
    vector<uev> uev_destinations;

    int total_voronoi_cells; //stores total number of voronoi cells alloted to a bot
    int current_behaviour; //0- Go to Boundary; 1- Cover Boundary; 2- Cover rest of the voronoi partition, used in voronoi partition based Algo's
    int preferred_x; int preferred_y;//the preferred direction for go-to-boundary behaviour in voronoi cells
    vector <bp> boundary_points;//boundary points of voronoi partition
    bool boundary_lost;//this variable gets true when we are covering the boundary of voronoi partition and it gets lost due to some obstacle
    int voronoi_wall;
    vector<uv> uv_boundary;//unvisited boundary cells in voronoi based algos
    int unvisited_discovered_boundary;//keeps the count of unvisited covered boundaries
    std::vector<std::vector<pair<int, int>> > patches;//stores all the patches and there points

    //these are only required in simulation;
    int index_travelled;

    //required_for_MDFS and also to mark ending of Algorithms
    bool coverage_completed;
    int start_cell_x;
    int start_cell_y;

    //required to simulate delays in movement
    int current_orient;
    int last_orient;
    int iter_wait;//0 initially, using iterations
    bool movement_made;//1 initially
    double last_move_time; //0 initially, using time
    double wait_time;//for delay using time
    double time_spent_in_computation;
    int way_to_move;//0-go straight, 1-turn 90 degree, 2, turn 180 degree
    double bot_start_movement;
    double bot_end_movement;

   //required in S-MSTC
    stack<pair<int,int> > second_sk;

   //requiedfor collisionavoidance in simulation
   bool wait_to_plan;

   double path_completion_time; //used to compute completion time;

   double comm_dist; //communication distance in fts.
       
     
   

    PathPlannerGrid(int csx,int csy,int th,std::vector<std::vector<nd> > &wg):cell_size_x(csx),cell_size_y(csy),threshold_value(th),total_points(0),start_grid_x(-1),start_grid_y(-1),goal_grid_x(-1),goal_grid_y(-1),robot_id(-1),goal_id(-1),origin_id(-1),robot_tag_id(-1),world_grid(wg), last_grid_x(-1), last_grid_y(-1), next_target_index(0), deadlock_check_counter(0), total_voronoi_cells(0), current_behaviour(0), preferred_x(0), preferred_y(0), boundary_lost(0), voronoi_wall(2), unvisited_discovered_boundary(0), index_travelled(0), coverage_completed(0), start_cell_x(-1), start_cell_y(-1), current_orient(-1), last_orient(-1), iter_wait(0), movement_made(1), wait_to_plan(0), last_move_time(0), time_spent_in_computation(0), way_to_move(0), path_completion_time(0), bot_start_movement(0), bot_end_movement(0), deadlock_replanned(0), comm_dist(50){
      initializeLocalPreferenceMatrix();
      initializeBactrackSearchMatrix();
      path_color = cv::Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
      first_call = 1;
      status = 0;
      target_grid_cell = make_pair(-1, -1);		
    }
    PathPlannerGrid(std::vector<std::vector<nd> > &wg):total_points(0),start_grid_x(-1),start_grid_y(-1),goal_grid_x(-1),goal_grid_y(-1),robot_id(-1),goal_id(-1),origin_id(-1),robot_tag_id(-1),world_grid(wg),last_grid_x(-1), last_grid_y(-1), next_target_index(0), deadlock_check_counter(0), total_voronoi_cells(0), current_behaviour(0), preferred_x(0), preferred_y(0), boundary_lost(0), voronoi_wall(2), unvisited_discovered_boundary(0), index_travelled(0), coverage_completed(0), start_cell_x(-1), start_cell_y(-1), current_orient(-1), last_orient(-1), iter_wait(0), movement_made(1), wait_to_plan(0), last_move_time(0), time_spent_in_computation(0), way_to_move(0), path_completion_time(0), bot_start_movement(0), bot_end_movement(0), deadlock_replanned(0), comm_dist(50){
      initializeLocalPreferenceMatrix();
      initializeBactrackSearchMatrix();
      path_color = cv::Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
      first_call = 1;
      status = 0;
      target_grid_cell = make_pair(-1, -1);
    }


    PathPlannerGrid& operator=(const PathPlannerGrid& pt){
      path_color = pt.path_color;
      goal_id = pt.goal_id;
      robot_id = pt.robot_id;
      origin_id = pt.origin_id;
      robot_tag_id = pt.robot_tag_id;
      cell_size_x = pt.cell_size_x;
      cell_size_y = pt.cell_size_y;
      threshold_value = pt.threshold_value;
      path_points = pt.path_points;
      pixel_path_points = pt.pixel_path_points;
      total_points = pt.total_points;
      start_grid_x = pt.start_grid_x,start_grid_y = pt.start_grid_y;
      goal_grid_x = pt.goal_grid_x, goal_grid_y = pt.goal_grid_y;
      rcells = pt.rcells, ccells = pt.ccells;
      world_grid = pt.world_grid;
      //no need to copy aj as all are the same
      sk = pt.sk;
      first_call = pt.first_call;
      bt_destinations = pt.bt_destinations;
      uev_destinations = pt.uev_destinations;
      boundary_points = pt.boundary_points;
      uv_boundary = pt.uv_boundary;
      robot_tag_id = pt.robot_tag_id;
      total_voronoi_cells = pt.total_voronoi_cells;
      index_travelled = pt.index_travelled;
      return *this;
    }
    double distance(double x1,double y1,double x2,double y2);
    void shareMap(AprilInterfaceAndVideoCapture &testbed, vector<PathPlannerGrid> &bots);
    void initializeLocalPreferenceMatrix();
    //invert visitable and non visitable cells
    void gridInversion(const PathPlannerGrid &planner,int rid);
    void addPoint(int ind,int px, int py, int x,int y);
    //criteria based on which to decide whether cell is empty
    bool isEmpty(int r,int c);
    bool isFellowAgent(int x,int y,std::vector<AprilTags::TagDetection> &detections);
    bool pixelIsInsideTag(int x,int y,std::vector<AprilTags::TagDetection> &detections,int ind);
    int setRobotCellCoordinates(std::vector<AprilTags::TagDetection> &detections);
    int setGoalCellCoordinates(std::vector<AprilTags::TagDetection> &detections);
    void drawGrid(cv::Mat &image, vector<PathPlannerGrid> &bots);
    //image rows and columns are provided
    void initializeGrid(int r,int c);
    //check for obstacles but excludes the black pixels obtained from apriltags
    void overlayGrid(std::vector<AprilTags::TagDetection> &detections,cv::Mat &grayImage);
    //find shortest traversal,populate path_points
    void findshortest(AprilInterfaceAndVideoCapture &testbed);
    std::pair<int,int> setParentUsingOrientation(robot_pose &ps);
    void addGridCellToPath(int r,int c,AprilInterfaceAndVideoCapture &testbed);
    bool isBlocked(int ngr, int ngc);
    int getWallReference(int r,int c,int pr, int pc);
    void addBacktrackPointToStackAndPath(std::stack<std::pair<int,int> > &sk,std::vector<std::pair<int,int> > &incumbent_cells,int &ic_no,int ngr, int ngc,std::pair<int,int> &t,AprilInterfaceAndVideoCapture &testbed);
    void BSACoverage(AprilInterfaceAndVideoCapture &testbed,robot_pose &ps);
    int backtrackSimulateBid(std::pair<int,int> target,AprilInterfaceAndVideoCapture &testbed);
    void BSACoverageIncremental(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps,double reach_distance,vector<PathPlannerGrid> &bots);
    void findCoverageLocalNeighborPreference(AprilInterfaceAndVideoCapture &testbed,robot_pose &ps);
    void findCoverageGlobalNeighborPreference(AprilInterfaceAndVideoCapture &testbed);
    void drawPath(cv::Mat &image);
    void DeadlockReplan(AprilInterfaceAndVideoCapture &testbed, vector<PathPlannerGrid> &bots);
    //following 3 functions were added for the updated backtrack search Algo
    int checkBactrackCondition(pair<int, int> p1, pair <int, int> p2);
    bool checkBactrackValidityForBoB(pair <int, int> t);
    bool bactrackValidityForBSA_CM(pair <int, int> t, int nx, int ny, int j);
    bool bactrackValidityForBoB(pair <int, int> t, int nx, int ny, int j);
    void SSB(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);
    void initializeBactrackSearchMatrix();//to initialize the matrix which help in backtrack search, namely blockedcellcheck
    void BoustrophedonMotionWithUpdatedBactrackSelection(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);
    int backtrackSimulateBidForBoustrophedonMotion(pair<int,int> target,AprilInterfaceAndVideoCapture &testbed);
    void BoustrophedonMotionSearchForBTAmongstUEV(AprilInterfaceAndVideoCapture &testbed, vector<PathPlannerGrid> &bots, std::vector<std::pair<int,int> > &incumbent_cells, int ic_no, std::stack<std::pair<int,int> > &sk);

    void BSA_CMSearchForBTAmongstUEV(AprilInterfaceAndVideoCapture &testbed, vector<PathPlannerGrid> &bots, vector<pair<int,int> > &incumbent_cells, int ic_no, stack<pair<int,int> > &sk);

    void BoustrophedonMotionWithBSA_CMlikeBacktracking(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);

    void setPathColor();//to set the color of path

    void defineVoronoiPartition(AprilInterfaceAndVideoCapture &testbed, vector<PathPlannerGrid> &bots);

    void VoronoiPartitionBasedOnlineCoverage(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);

    void discoverPatch(int patch_count, int r, int c);
    void repartition(vector<PathPlannerGrid> &bots);
    void drawRobot(cv::Mat &image);
 
    bool checkReachStatus(pair <int, int> t, robot_pose &ps, double reach_distance);

    void BoB(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots); 

    void MDFS(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots); 

    void ANTS(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);  

    void S_MSTC(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots); 
	
    void BrickAndMortar(AprilInterfaceAndVideoCapture &testbed, robot_pose &ps, double reach_distance, vector<PathPlannerGrid> &bots);   

    bool checkConnectivity(pair <int, int> start, pair <int, int> end);//required to checking blockage in Brick and Mortar;

    void updateMovementinSimulation(AprilInterfaceAndVideoCapture &testbed);

    pair <int, int> bidForBT(AprilInterfaceAndVideoCapture &testbed, pair <int, int> current_cell, vector<PathPlannerGrid> &bots);

    pair <int, int> bidForUEV(AprilInterfaceAndVideoCapture &testbed, pair <int, int> current_cell, vector<PathPlannerGrid> &bots);

    pair <int, int> nearestBT(AprilInterfaceAndVideoCapture &testbed, pair <int, int> current_cell, vector<PathPlannerGrid> &bots);
   
   
    
};

class PathPlannerUser{
  public:
    std::vector<pt> path_points;
    std::vector<std::pair<int,int> > pixel_path_points;
    int total_points;
    AprilInterfaceAndVideoCapture *testbed;
    PathPlannerUser(AprilInterfaceAndVideoCapture *tb):total_points(0),testbed(tb){}
    void addPoint(int px, int py, double x,double y);
    void CallBackFunc(int event, int x, int y);
    void drawPath(cv::Mat &image);
    static void CallBackFunc(int event, int x, int y, int flags, void* userdata){
      PathPlannerUser *ppu = reinterpret_cast<PathPlannerUser*>(userdata);
      ppu->CallBackFunc(event,x,y);
    }
};

#endif
