#ifndef CDPR_H
#define CDPR_H

#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <gazebo_msgs/LinkState.h>
#include <geometry_msgs/Pose.h>
#include <visp/vpHomogeneousMatrix.h>

class CDPR
{
public:
    CDPR(ros::NodeHandle &_nh);

    inline bool ok() {return cables_ok && platform_ok;}

    inline void getPose(vpHomogeneousMatrix &M) {M = M_;}
    inline void getVelocity(vpColVector &v) {v = v_;}
    inline void getDesiredPose(vpHomogeneousMatrix &M) {M = Md_;}
    inline vpPoseVector getPoseError() {return vpPoseVector(M_.inverse()*Md_);}

    void sendTensions(vpColVector &f);

    // get model parameters
    inline unsigned int n_cables() {return n_cable;}
    inline double mass() {return mass_;}
    inline vpMatrix inertia() {return inertia_;}
    inline void tensionMinMax(double &fmin, double &fmax) {fmin = f_min; fmax = f_max;}

    // structure matrix
    void computeW(vpMatrix &W);



protected:
    // subscriber to gazebo data
    ros::Subscriber cables_sub, platform_sub;
    bool cables_ok, platform_ok;
    sensor_msgs::JointState cable_states;

    // subscriber to desired pose
    ros::Subscriber setpoint_sub;

    // publisher to tensions
    ros::Publisher tensions_pub;
    sensor_msgs::JointState tensions_msg;

    // pf pose and velocity
    vpHomogeneousMatrix M_, Md_;
    vpColVector v_;

    // model data
    double mass_, f_min, f_max;
    vpMatrix inertia_;
    std::vector<vpTranslationVector> Pf, Pp;
    unsigned int n_cable;


    // callbacks
    // callback for platform state
    void PFState_cb(const gazebo_msgs::LinkStateConstPtr &_msg)
    {
        platform_ok = true;
        M_.insert(vpTranslationVector(_msg->pose.position.x, _msg->pose.position.y, _msg->pose.position.z));
        M_.insert(vpQuaternionVector(_msg->pose.orientation.x, _msg->pose.orientation.y, _msg->pose.orientation.z,_msg->pose.orientation.w));
    }

    // callback for pose setpoint
    void Setpoint_cb(const geometry_msgs::PoseConstPtr &_msg)
    {
        Md_.insert(vpTranslationVector(_msg->position.x, _msg->position.y, _msg->position.z));
        Md_.insert(vpQuaternionVector(_msg->orientation.x, _msg->orientation.y, _msg->orientation.z,_msg->orientation.w));
    }

    // callback for cable states
    void Cables_cb(const sensor_msgs::JointState &_msg)
    {
        cables_ok = true;
        cable_states = _msg;
    }
};

#endif // CDPR_H