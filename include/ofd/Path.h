#ifndef __OFD_PATH_H__
#define __OFD_PATH_H__

#include <memory>
#include <vector>

using namespace std;

namespace ofd{

    // **************** struct Point ****************
    typedef struct Point{
        double x;
        double y;

        Point() : x(0.0), y(0.0){};
        Point(double _x, double _y): x(_x), y(_y){};

        void Clear(){
            x = y = 0.0;
        }

        void Offset(double dx, double dy){
            x += dx; y += dy;
        }

        bool operator ==(const Point& other) const {
            return x == other.x && y == other.y;
        }

        bool operator !=(const Point& other) const {
            return !(*this == other);
        }

    } *Point_t;

    class Subpath;
    typedef shared_ptr<Subpath> SubpathPtr;

    // **************** class Subpath ****************
    class Subpath{
        public:
            Subpath(const Point &startPoint);
            Subpath(const Subpath* other);

            // ================ Public Methods ================
        public:
            SubpathPtr Clone() const;

            void ClosePath();
            bool IsClosed() const{return m_bClosed;};

            size_t GetNumPoints() const{return m_points.size();};
            const Point& GetFirstPoint() const;
            const Point& GetLastPoint() const; 
            const Point& GetPoint(size_t idx) const;
            void SetPoint(size_t idx, const Point& point);

            void Offset(double dx, double dy);
            void LineTo(const Point& point);
            void CurveTo(const Point& p0, const Point& p1, const Point& p2);

        private:
            vector<Point> m_points;
            vector<bool> m_curves;
            bool m_bClosed;

    }; // class Subpath

    class Path;
    typedef shared_ptr<Path> PathPtr;

    // **************** class Path ****************
    class Path{
        public:
            Path();
            ~Path();

            // ================ Public Methods ================
        public:
            void MoveTo(const Point& startPoint);
            void LineTo(const Point& point);
            void CurveTo(const Point& p0, const Point& p1, const Point& p2);
            void ClosePath();
            void Offset(double dx, double dy);
            void Append(const Path& otherPath);

            size_t GetNumSubpaths() const {return m_subpaths.size();};
            SubpathPtr GetSubpath(size_t idx) const {return m_subpaths[idx];};
            SubpathPtr GetLastSubpath() const;

            const Point& GetStartPoint() const {return m_startPoint;};
        private:
            bool m_bJustMoved;
            Point m_startPoint;
            vector<SubpathPtr> m_subpaths;

    }; // class Path

}; // namespace ofd

#endif // __OFD_PATH_H__
