#pragma once
/*********************************************************
* Package Overview                                       *
* (O)beject (O)rientated (B)oundingbox uses pca to build *
* an object aligned bounding box                         *
*********************************************************/
namespace Euclid
{

template<typename Polyhedron_3>
class OBB
{
	using FT = typename Polyhedron_3::Traits::Kernel::FT;
	using Vec3 = Eigen::Matrix<FT, 3, 1>;
	using Mat3 = Eigen::Matrix<FT, 3, 3>;

public:
	explicit OBB(const Polyhedron_3& mesh);
	explicit OBB(const std::vector<Vec3>& vertices);
	~OBB();

	Vec3 center() const;
	std::array<Vec3, 3> directions() const;
	Vec3 halfsize() const;
	float radius() const;
	Vec3 lbb() const;
	Vec3 lbf() const;
	Vec3 ltb() const;
	Vec3 ltf() const;
	Vec3 rbb() const;
	Vec3 rbf() const;
	Vec3 rtb() const;
	Vec3 rtf() const;

private:
	Mat3 _directions;
	Vec3 _center;
	Vec3 _halfsize;
};

} // namespace Euclid

#include "src/OBB.cpp"
