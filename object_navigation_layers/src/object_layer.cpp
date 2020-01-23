#include <object_layers/object_layer.h>
#include <pluginlib/class_list_macros.h>

PLUGINLIB_EXPORT_CLASS(object_navigation_layers::ObjectLayer, costmap_2d::Layer)

using costmap_2d::LETHAL_OBSTACLE;
using costmap_2d::NO_INFORMATION;

namespace object_navigation_layers
{

ObjectLayer::ObjectLayer() {}

void ObjectLayer::onInitialize()
{
  ros::NodeHandle nh("~/" + name_);
  current_ = true;
  default_value_ = NO_INFORMATION;
  matchSize();

  dsrv_ = new dynamic_reconfigure::Server<object_navigation_layers::ObjectLayerConfig>(nh);
  dynamic_reconfigure::Server<object_navigation_layers::ObjectLayerConfig>::CallbackType cb = boost::bind(
      &ObjectLayer::reconfigureCB, this, _1, _2);
  dsrv_->setCallback(cb);

  cloud_sub_ = nh.subscribe("/objects_pointcloud/output", 1, &ObjectLayer::pointCloudCallback, this);
}


void ObjectLayer::matchSize()
{
  Costmap2D* master = layered_costmap_->getCostmap();
  resizeMap(master->getSizeInCellsX(), master->getSizeInCellsY(), master->getResolution(),
            master->getOriginX(), master->getOriginY());
}


void ObjectLayer::reconfigureCB(object_navigation_layers::ObjectLayerConfig &config, uint32_t level)
{
  enabled_ = config.enabled;
}

void ObjectLayer::pointCloudCallback(const sensor_msgs::PointCloud2::ConstPtr& msg)
{
  cloud_.reset(new pcl::PointCloud<pcl::PointXYZ>);
  pcl::fromROSMsg(*msg, *cloud_);
}

void ObjectLayer::updateBounds(double robot_x, double robot_y, double robot_yaw, double* min_x,
                                           double* min_y, double* max_x, double* max_y)
{
  if (!enabled_)
    return;

  if (!cloud_)
    return;

  pcl::PointXYZ pp;
  double mark_x, mark_y;
  unsigned int mx;
  unsigned int my;

  for (int i = 0; i < cloud_->size(); i++) {
    pp = cloud_->points[i];

    mark_x = pp.x;
    mark_y = pp.y;

    // if(worldToMap(mark_x, mark_y, mx, my)) {
    //   setCost(mx, my, LETHAL_OBSTACLE);
    // }

    *min_x = std::min(*min_x, mark_x);
    *min_y = std::min(*min_y, mark_y);
    *max_x = std::max(*max_x, mark_x);
    *max_y = std::max(*max_y, mark_y);
  }

}

void ObjectLayer::updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, int max_i,
                                          int max_j)
{
  if (!enabled_)
    return;

  if (!cloud_)
    return;

  // setCost is for set "current" cost, not for "accumulated" cost
  // To make accumulated cost map, use grid map ex) CostmapLayer::updateWithMax

  pcl::PointXYZ pp;
  double mark_x, mark_y;
  unsigned int mx;
  unsigned int my;

  for (int i = 0; i < cloud_->size(); i++) {
    pp = cloud_->points[i];
    mark_x = pp.x;
    mark_y = pp.y;
    if(master_grid.worldToMap(mark_x, mark_y, mx, my)) {
      master_grid.setCost(mx, my, LETHAL_OBSTACLE);
    }
  }

  // unsigned char* master_array = master_grid.getCharMap();
  // unsigned int span = master_grid.getSizeInCellsX();

  // for (int j = min_j; j < max_j; j++)
  // {
  //   unsigned int it = j * span + min_i;
  //   for (int i = min_i; i < max_i; i++)
  //   {
  //     if (costmap_[it] == NO_INFORMATION){
  //       it++;
  //       continue;
  //     }

  //     unsigned char old_cost = master_array[it];
  //     if (old_cost == NO_INFORMATION || old_cost < costmap_[it])
  //       master_array[it] = costmap_[it];
  //     it++;
  //   }
  // }

}

} // end namespace
