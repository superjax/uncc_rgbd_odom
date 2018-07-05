/*
 * RobustMatcher.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: eriba
 */

#include <time.h>

#include <opencv2/features2d/features2d.hpp>

#include <rgbd_odometry/RobustMatcher.h>

RobustMatcher::~RobustMatcher() {
    // TODO Auto-generated destructor stub
}

int RobustMatcher::ratioTest(std::vector<std::vector<cv::DMatch> > &matches) {
    int removed = 0;
    // for all matches
    for (std::vector<std::vector<cv::DMatch> >::iterator matchIterator = matches.begin();
            matchIterator != matches.end(); ++matchIterator) {
        // if 2 NN has been identified
        if (matchIterator->size() > 1) {
            // check distance ratio
            if ((*matchIterator)[0].distance / (*matchIterator)[1].distance > ratio_) {
                matchIterator->clear(); // remove match
                removed++;
            }
        } else { // does not have 2 neighbours
            matchIterator->clear(); // remove match
            removed++;
        }
    }
    return removed;
}

void RobustMatcher::symmetryTest(const std::vector<std::vector<cv::DMatch> >& matches1,
        const std::vector<std::vector<cv::DMatch> >& matches2,
        std::vector<cv::DMatch>& symMatches) {

    // for all matches image 1 -> image 2
    for (std::vector<std::vector<cv::DMatch> >::const_iterator
        matchIterator1 = matches1.begin(); matchIterator1 != matches1.end(); ++matchIterator1) {

        // ignore deleted matches
        if (matchIterator1->empty() || matchIterator1->size() < 2)
            continue;

        // for all matches image 2 -> image 1
        for (std::vector<std::vector<cv::DMatch> >::const_iterator
            matchIterator2 = matches2.begin(); matchIterator2 != matches2.end(); ++matchIterator2) {
            // ignore deleted matches
            if (matchIterator2->empty() || matchIterator2->size() < 2)
                continue;

            // Match symmetry test
            if ((*matchIterator1)[0].queryIdx ==
                    (*matchIterator2)[0].trainIdx &&
                    (*matchIterator2)[0].queryIdx ==
                    (*matchIterator1)[0].trainIdx) {
                // add symmetrical match
                symMatches.push_back(
                        cv::DMatch((*matchIterator1)[0].queryIdx,
                        (*matchIterator1)[0].trainIdx,
                        (*matchIterator1)[0].distance));
                break; // next match in image 1 -> image 2
            }
        }
    }

}

void RobustMatcher::robustMatch(std::vector<cv::DMatch>& good_matches,
        const cv::Mat& descriptors_frame,
        const cv::Mat& descriptors_model) {

    // 2. Match the image descriptors
    std::vector<std::vector<cv::DMatch> > matches12, matches21;

    // 2a. From image 1 to image 2
    matcher_->knnMatch(descriptors_frame, descriptors_model, matches12, 2); // return 2 nearest neighbors

    // 2b. From image 2 to image 1
    matcher_->knnMatch(descriptors_model, descriptors_frame, matches21, 2); // return 2 nearest neighbors

    // 3. Remove matches for which NN ratio is > than threshold
    // clean image 1 -> image 2 matches
    ratioTest(matches12);
    // clean image 2 -> image 1 matches
    ratioTest(matches21);

    // 4. Remove non-symmetrical matches
    symmetryTest(matches12, matches21, good_matches);

}

void RobustMatcher::fastRobustMatch(std::vector<cv::DMatch>& good_matches,
        const cv::Mat& descriptors_frame,
        const cv::Mat& descriptors_model) {
    good_matches.clear();

    // 2. Match the image descriptors
    std::vector<std::vector<cv::DMatch> > matches;
    matcher_->knnMatch(descriptors_frame, descriptors_model, matches, 2);
    // 3. Remove matches for which NN ratio is > than threshold
    ratioTest(matches);

    // 4. Fill good matches container
    for (std::vector<std::vector<cv::DMatch> >::iterator
        matchIterator = matches.begin(); matchIterator != matches.end(); ++matchIterator) {
        if (!matchIterator->empty()) {
            good_matches.push_back((*matchIterator)[0]);
        }
    }

}
