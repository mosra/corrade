class Corrade < Formula
  desc "C++11/C++14 multiplatform utility library"
  homepage "https://magnum.graphics/corrade/"
  url "https://github.com/mosra/corrade/archive/v2019.10.tar.gz"
  # wget https://github.com/mosra/corrade/archive/v2019.10.tar.gz -O - | sha256sum
  sha256 "19dbf3c0b28a06a7017d627ee7b84c23b994c469198c1134a8aeba9cfbff7ec3"
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
