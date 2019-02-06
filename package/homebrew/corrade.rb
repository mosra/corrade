class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2018.10.tar.gz"
  sha256 "a1aea381e512da2ae3b92687bdc7b20152a773f1bb64f36f31cd6b3cef9652a5"
  head "git://github.com/mosra/corrade.git"

  depends_on "cmake"

  def install
    system "mkdir build"
    cd "build" do
      system "cmake", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_INSTALL_PREFIX=#{prefix}", ".."
      system "cmake", "--build", "."
      system "cmake", "--build", ".", "--target", "install"
    end
  end
end
