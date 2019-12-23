//
// Created by phmoll on 12/23/19.
//

#ifndef QUADTREESYNCEVALUATION_GZIP_H
#define QUADTREESYNCEVALUATION_GZIP_H

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

class GZip {
public:
    static std::string compress(const std::string& data, boost::iostreams::gzip_params params=boost::iostreams::gzip_params(boost::iostreams::gzip::best_speed))
    {
        namespace bio = boost::iostreams;

        std::stringstream compressed;
        std::stringstream origin(data);

        bio::filtering_streambuf<bio::input> out;
        out.push(bio::gzip_compressor(params));
        out.push(origin);
        bio::copy(out, compressed);

        return compressed.str();
    }

    static std::string decompress(const std::string& data)
    {
        namespace bio = boost::iostreams;

        std::stringstream compressed(data);
        std::stringstream decompressed;

        bio::filtering_streambuf<bio::input> out;
        out.push(bio::gzip_decompressor());
        out.push(compressed);
        bio::copy(out, decompressed);

        return decompressed.str();
    }
};


#endif //QUADTREESYNCEVALUATION_GZIP_H
