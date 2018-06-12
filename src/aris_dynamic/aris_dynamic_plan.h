﻿#ifndef ARIS_DYNAMIC_PLAN_H_
#define ARIS_DYNAMIC_PLAN_H_

#include <list>
#include <cmath>
#include <iostream>

#include <aris_dynamic_model.h>
#include <aris_dynamic_matrix.h>

namespace aris
{
	/// \brief 轨迹规划命名空间
	/// \ingroup aris
	/// 
	///
	///
	namespace dynamic
	{
		auto inline acc_up(int n, int i)noexcept->double{return (-1.0 / 2 / n / n / n * i*i*i + 3.0 / 2.0 / n / n * i*i);}
		auto inline acc_down(int n, int i)noexcept->double{	return (-1.0*i*i*i / 2.0 / n / n / n + 3.0 * i*i / 2.0 / n / n);}
		auto inline dec_up(int n, int i)noexcept->double{return 1.0 - (-1.0 / 2.0 / n / n / n * (n - i)*(n - i)*(n - i) + 3.0 / 2.0 / n / n * (n - i)*(n - i));}
		auto inline dec_down(int n, int i)noexcept->double{	return 1.0 - (-1.0*(n - i)*(n - i)*(n - i) / 2.0 / n / n / n + 3.0 * (n - i)*(n - i) / 2.0 / n / n);}

		auto inline acc_even(int n, int i)noexcept->double{	return 1.0 / n / n  * i * i;}
		auto inline dec_even(int n, int i)noexcept->double{	return 1.0 - 1.0 / n / n * (n - i)*(n - i);	}
		auto inline even(int n, int i)noexcept->double{	return 1.0 / n*i;}

		auto inline s_p2p(int n, int i, double begin_pos, double end_pos)noexcept->double
		{	
			double a = 4.0 * (end_pos - begin_pos) / n / n;
			return i <= n / 2.0 ? 0.5*a*i*i + begin_pos : end_pos - 0.5*a*(n - i)*(n - i);
		}
		auto inline s_v2v(int n, int i, double begin_vel, double end_vel)noexcept->double
		{
			double s = static_cast<double>(i) / n;
			double m = 1.0 - s;

			return (s*s*s - s*s)*end_vel*n + (m*m - m*m*m)*begin_vel*n;
		}
		auto inline s_interp(int n, int i, double begin_pos, double end_pos, double begin_vel, double end_vel)noexcept->double
		{
			double s = static_cast<double>(i) / n;
			
			double a, b, c, d;

			c = begin_vel*n;
			d = begin_pos;
			a = end_vel*n - 2.0 * end_pos + c + 2.0 * d;
			b = end_pos - c - d - a;

			return a*s*s*s+b*s*s+c*s+d;
		}

		auto moveAbsolute(Size i, double begin_pos, double end_pos, double vel, double acc, double dec, double &current_pos, double &current_vel, double &current_acc, Size& total_count)->void;


		using PathPlan = std::function<void(double s, double *pm, double *vs_over_s, double *as_over_s)>;
		class OptimalTrajectory :public Element
		{
		public:
			struct MotionLimit
			{
				double max_vel, min_vel, max_acc, min_acc, max_tor, min_tor, max_jerk, min_jerk;
			};
			struct Node
			{
				double time, s, ds, dds;
			};

			template <typename LimitArray>
			auto setMotionLimit(LimitArray limits)->void
			{
				motor_limits.clear();
				for (auto &limit : limits)
				{
					motor_limits.push_back(limit);
				}
			}
			auto setBeginNode(Node node)->void { beg_ = node; }
			auto setEndNode(Node node)->void { end_ = node; }
			auto setFunction(const PathPlan &path_plan)->void { this->plan = path_plan; }
			auto setSolver(UniversalSolver *solver)->void { this->solver = solver; }
			auto result()->std::vector<double>& { return result_; }
			auto run()->void;

			virtual ~OptimalTrajectory() = default;
			explicit OptimalTrajectory(const std::string &name = "optimal_trajectory") : Element(name) {}
			OptimalTrajectory(const OptimalTrajectory&) = default;
			OptimalTrajectory(OptimalTrajectory&&) = default;
			OptimalTrajectory& operator=(const OptimalTrajectory&) = default;
			OptimalTrajectory& operator=(OptimalTrajectory&&) = default;

		public:
			auto testForward()->void;
			auto join()->void;
			auto cptDdsConstraint(double s, double ds, double &max_dds, double &min_dds)->bool;
			auto cptInverseJacobi()->void;



			double failed_s;

			PathPlan plan;
			UniversalSolver *solver;

			Node beg_, end_;
			std::list<Node> list;
			std::list<Node>::iterator l_beg_, l_;
			std::vector<MotionLimit> motor_limits;

			std::vector<double> Ji_data_;

			std::vector<double> result_;

			
		};


		class FastPath
		{
		public:
			struct MotionLimit
			{
				double maxVel, minVel, maxAcc, minAcc;
			};
			struct Node
			{
				double time, s, ds, dds;
				bool isAccelerating;
			};
			struct Data
			{
				double * const Ji;
				double * const dJi;
				double * const Cv;
				double * const Ca;
				double * const g;
				double * const h;
				const Size size;
				double time, s, ds;
				double dsLhs, dsRhs;
				double ddsLhs, ddsRhs;
			};

			template <typename LimitArray>
			auto setMotionLimit(LimitArray limits)->void
			{
				motor_limits.clear();
				for (auto &limit : limits)
				{
					motor_limits.push_back(limit);
				}
			}
			auto setBeginNode(Node node)->void { beginNode=node; };
			auto setEndNode(Node node)->void { endNode = node; };
			auto setFunction(std::function<void(FastPath::Data &)> getEveryThing)->void { this->getEveryThing = getEveryThing; };
			auto result()->std::vector<double>& { return resultVec; };
			auto run()->void;
			

			FastPath() = default;
			~FastPath() = default;

		private:
			bool computeDsBundPure(FastPath::Data &data, std::vector<FastPath::MotionLimit> &limits);
			bool computeDdsBundPure(FastPath::Data &data, std::vector<FastPath::MotionLimit> &limits);
			bool computeDsBund(FastPath::Data &data, std::vector<FastPath::MotionLimit> &limits);
			bool computeDdsBund(FastPath::Data &data, std::vector<FastPath::MotionLimit> &limits);
			
			int computeForward(std::list<Node>::iterator iter, FastPath::Data &data, int num);
			int computeBackward(std::list<Node>::iterator iter, FastPath::Data &data, int num);
			bool compute(std::list<Node>::iterator iter, FastPath::Data &data);

			void concate(FastPath::Data &data);
		public:
			Node beginNode, endNode;
			std::list<Node> list;
			std::list<Node>::iterator finalIter;
			std::vector<MotionLimit> motor_limits;
			std::function<void(FastPath::Data &)> getEveryThing;

			std::vector<double> resultVec;
		};
	}
}


#endif