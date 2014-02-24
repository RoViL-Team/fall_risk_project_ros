#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
// PCL specific includes
#include <pcl/conversions.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>

ros::Publisher pub;

void
cloud_cb (const sensor_msgs::PointCloud2ConstPtr& input)
{           
    pcl::PCLPointCloud2 input_pcl;
    pcl::PCLPointCloud2 output_pcl;
    sensor_msgs::PointCloud2 output;
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_pass_filtered (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_outlier_filtered (new pcl::PointCloud<pcl::PointXYZ>);

    pcl_conversions::toPCL(*input, input_pcl);
    pcl::fromPCLPointCloud2(input_pcl, *cloud);
//    pcl::fromROSMsg (*input, *cloud);   //deprecated method to do conversion

    std::cerr << "Cloud before filtering: " << std::endl;
    std::cerr << *cloud << std::endl;

    // PassThrough Filter
    pcl::PassThrough<pcl::PointXYZ> pass;
    pass.setInputCloud (cloud);
    pass.setFilterFieldName ("z");
    pass.setFilterLimits (0.0, 5.0);
    //pass.setFilterLimitsNegative (true);
    pass.filter (*cloud_pass_filtered);

    // StatisticalOutlierRemoval Filter
    pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
    sor.setInputCloud (cloud_pass_filtered);
    sor.setMeanK (50);
    sor.setStddevMulThresh (1.0);
    sor.filter (*cloud_outlier_filtered);

    std::cerr << "Cloud after filtering: " << std::endl;
    std::cerr << *cloud_outlier_filtered << std::endl;

    pcl::toPCLPointCloud2(*cloud_outlier_filtered,output_pcl);
    pcl_conversions::fromPCL(output_pcl,output);
//    pcl::toROSMsg(*cloud_filtered,output);   //deprecated method to do conversion

    // Publish the data
    pub.publish (output);
}

int
main (int argc, char** argv)
{
  // Initialize ROS
  ros::init (argc, argv, "kinect_pcl_filter");
  ros::NodeHandle nh;

  // Create a ROS subscriber for the input point cloud
  ros::Subscriber sub = nh.subscribe ("/camera/depth/points", 1, cloud_cb);

  // Create a ROS publisher for the output point cloud
  pub = nh.advertise<sensor_msgs::PointCloud2> ("/camera/depth/filtered_points", 1);

  // Spin
  ros::spin ();
}