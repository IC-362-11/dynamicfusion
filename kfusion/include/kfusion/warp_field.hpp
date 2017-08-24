#ifndef KFUSION_WARP_FIELD_HPP
#define KFUSION_WARP_FIELD_HPP

/**
 * \brief
 * \details
 */
#include <dual_quaternion.hpp>
#include <kfusion/types.hpp>
#include <nanoflann/nanoflann.hpp>
#include <knn_point_cloud.hpp>
#include <kfusion/cuda/tsdf_volume.hpp>
#define KNN_NEIGHBOURS 8

namespace kfusion
{
    typedef nanoflann::KDTreeSingleIndexAdaptor<
            nanoflann::L2_Adaptor<float, utils::PointCloud>,
            utils::PointCloud,
            3 /* dim */
    > kd_tree_t;


    //    TODO: remember to rewrite this with proper doxygen formatting (e.g <sub></sub> rather than _
    /*!
     * \struct node
     * \brief A node of the warp field
     * \details The state of the warp field Wt at time t is defined by the values of a set of n
     * deformation nodes Nt_warp = {dg_v, dg_w, dg_se3}_t. Here, this is represented as follows
     *
     * \var node::index
     * Index of the node in the canonical frame. Equivalent to dg_v
     *
     * \var node::transform
     * Transform from canonical point to warped point, equivalent to dg_se in the paper.
     *
     * \var node::weight
     * Equivalent to dg_w
     */
    struct deformation_node
    {
        Vec3f vertex;
        kfusion::utils::DualQuaternion<float> transform;
        float weight = 0;
        bool valid = true;
    };
    class WarpField
    {
    public:
        WarpField();
        ~WarpField();

        void init(const cv::Mat& first_frame, const cv::Mat& normals);
        void energy(const cuda::Cloud &frame,
                    const cuda::Normals &normals,
                    const Affine3f &pose,
                    const cuda::TsdfVolume &tsdfVolume,
                    const std::vector<std::pair<kfusion::utils::DualQuaternion<float>,
                            kfusion::utils::DualQuaternion<float>>> &edges
        );

        float energy_data(const std::vector<Vec3f> &warped_vertices,
                          const std::vector<Vec3f> &warped_normals,
                          const Intr &intr);
        void energy_reg(const std::vector<std::pair<kfusion::utils::DualQuaternion<float>,
                kfusion::utils::DualQuaternion<float>>> &edges);

        float tukeyPenalty(float x, float c = 4.685) const;

        float huberPenalty(float a, float delta) const;

        void warp(std::vector<Point, std::allocator<Point>>& points,
                  std::vector<Point, std::allocator<Point>>& normals) const;

        void warp(std::vector<Vec3f>& points, std::vector<Vec3f>& normals) const;
        void warp(cuda::Cloud& points) const;

        utils::DualQuaternion<float> DQB(const Vec3f& vertex) const;

        float weighting(float squared_dist, float weight) const;
        void KNN(Vec3f point, float K = KNN_NEIGHBOURS) const;

        void clear();

        const std::vector<deformation_node>* getNodes() const;
        const cv::Mat getNodesAsMat() const;
        void setWarpToLive(const Affine3f &pose);
        void insertNewNodes(const std::vector<Vec3f>& points, const std::vector<Vec3f>& normals);
        std::vector<float> latestSquared() const;
    private:
        //    FIXME: should be a pointer
        std::vector<deformation_node> nodes;
        kd_tree_t* index;
        Affine3f warp_to_live;
        void buildKDTree();
    };
}
#endif //KFUSION_WARP_FIELD_HPP
