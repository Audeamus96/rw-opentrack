#include "frame.hpp"

#include "compat/math.hpp"

#include <opencv2/imgproc.hpp>

namespace pt_module {

void Preview::set_last_frame(const pt_frame& frame_)
{
    const cv::Mat& frame2 = frame_.as_const<const Frame>()->mat;
    const cv::Mat* frame;

    if (frame2.channels() == 1)
    {
        frame_tmp.create(frame2.rows, frame2.cols, CV_8UC3);
        cv::cvtColor(frame2, frame_tmp, cv::COLOR_GRAY2BGR);
        frame = &frame_tmp;
    }
    else
        frame = &frame2;

    if (frame->channels() != 3)
    {
        eval_once(qDebug() << "tracker/pt: camera frame depth: 3 !=" << frame->channels());
        frame_copy.create(cv::Size{frame_out.cols, frame_out.rows}, CV_8UC3);
        frame_copy.setTo({0});
    }

    const bool need_resize = frame2.cols != frame_out.cols || frame2.rows != frame_out.rows;
    if (need_resize)
        cv::resize(frame2, frame_copy, cv::Size(frame_out.cols, frame_out.rows), 0, 0, cv::INTER_NEAREST);
    else
        frame->copyTo(frame_copy);
}

Preview::Preview(int w, int h)
{
    frame_out.create(w, h, CV_8UC4);
    frame_copy.create(w, h, CV_8UC3);
    frame_copy.setTo({0});
}

QImage Preview::get_bitmap()
{
    int stride = (int)frame_out.step.p[0];

    if (stride < frame_out.cols * 4)
    {
        eval_once(qDebug() << "bad stride" << stride
                           << "for bitmap size" << frame_copy.cols << frame_copy.rows);
        return QImage();
    }

    cv::cvtColor(frame_copy, frame_out, cv::COLOR_BGR2BGRA);

    return QImage((const unsigned char*) frame_out.data,
                  frame_out.cols, frame_out.rows,
                  stride,
                  QImage::Format_ARGB32);
}

void Preview::draw_head_center(f x, f y)
{
    auto [px_, py_] = to_pixel_pos(x, y, frame_copy.cols, frame_copy.rows);

    int px = iround(px_), py = iround(py_);

    constexpr int len = 9;

    static const cv::Scalar color(0, 255, 255);
    cv::line(frame_copy,
             cv::Point(px - len, py),
             cv::Point(px + len, py),
             color, 1);
    cv::line(frame_copy,
             cv::Point(px, py - len),
             cv::Point(px, py + len),
             color, 1);
}

} // ns pt_module
