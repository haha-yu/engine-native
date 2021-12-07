/****************************************************************************
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#include "network/HttpCookie.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include "platform/FileUtils.h"

void HttpCookie::readFile() {
    std::string inString = cc::FileUtils::getInstance()->getStringFromFile(_cookieFileName);
    if (!inString.empty()) {
        std::vector<std::string> cookiesVec;
        cookiesVec.clear();

        std::stringstream stream(inString);
        std::string       item;
        while (std::getline(stream, item, '\n')) {
            cookiesVec.push_back(item);
        }

        if (cookiesVec.empty()) {
            return;
        }

        _cookies.clear();

        for (auto &cookie : cookiesVec) {
            if (cookie.length() == 0) {
                continue;
            }

            if (cookie.find("#HttpOnly_") != std::string::npos) {
                cookie = cookie.substr(10);
            }

            if (cookie.at(0) == '#') {
                continue;
            }

            CookiesInfo              co;
            std::stringstream        streamInfo(cookie);
            std::vector<std::string> elems;
            std::string              elemsItem;

            while (std::getline(streamInfo, elemsItem, '\t')) {
                elems.push_back(elemsItem);
            }

            co.domain = elems[0];
            if (co.domain.at(0) == '.') {
                co.domain = co.domain.substr(1);
            }
            co.tailmatch = (strcmp("TRUE", elems[1].c_str()) != 0);
            co.path      = elems[2];
            co.secure    = (strcmp("TRUE", elems[3].c_str()) != 0);
            co.expires   = elems[4];
            co.name      = elems[5];
            co.value     = elems[6];
            _cookies.push_back(co);
        }
    }
}

const std::vector<CookiesInfo> *HttpCookie::getCookies() const {
    return &_cookies;
}

const CookiesInfo *HttpCookie::getMatchCookie(const std::string &url) const {
    for (const auto &cookie : _cookies) {
        if (url.find(cookie.domain) != std::string::npos) {
            return &cookie;
        }
    }

    return nullptr;
}

void HttpCookie::updateOrAddCookie(CookiesInfo *cookie) {
    for (auto &iter : _cookies) {
        if (cookie->domain == iter.domain) {
            iter = *cookie;
            return;
        }
    }
    _cookies.push_back(*cookie);
}

void HttpCookie::writeFile() {
    FILE *out;
    out = fopen(_cookieFileName.c_str(), "w");
    fputs(
        "# Netscape HTTP Cookie File\n"
        "# http://curl.haxx.se/docs/http-cookies.html\n"
        "# This file was generated by cocos2d-x! Edit at your own risk.\n"
        "# Test cocos2d-x cookie write.\n\n",
        out);

    std::string line;
    for (auto &cookie : _cookies) {
        line.clear();
        line.append(cookie.domain);
        line.append(1, '\t');
        cookie.tailmatch ? line.append("TRUE") : line.append("FALSE");
        line.append(1, '\t');
        line.append(cookie.path);
        line.append(1, '\t');
        cookie.secure ? line.append("TRUE") : line.append("FALSE");
        line.append(1, '\t');
        line.append(cookie.expires);
        line.append(1, '\t');
        line.append(cookie.name);
        line.append(1, '\t');
        line.append(cookie.value);
        //line.append(1, '\n');

        fprintf(out, "%s\n", line.c_str());
    }

    fclose(out);
}

void HttpCookie::setCookieFileName(const std::string &filename) {
    _cookieFileName = filename;
}
