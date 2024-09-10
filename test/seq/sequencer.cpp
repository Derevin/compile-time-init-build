#include <flow/flow.hpp>
#include <seq/builder.hpp>
#include <seq/impl.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
int attempt_count;
std::string result{};

using builder = flow::graph_builder<"test_seq", seq::impl>;
} // namespace

TEST_CASE("build and run empty seq", "[seq]") {
    auto g = seq::builder<>{};
    auto seq_impl = builder::build(g);
    REQUIRE(seq_impl.has_value());
    CHECK(seq_impl->forward() == seq::status::DONE);
    CHECK(seq_impl->backward() == seq::status::DONE);
}

TEST_CASE("build seq with one step and run forwards and backwards", "[seq]") {
    result.clear();

    auto s = seq::step<"S">(
        []() -> seq::status {
            result += "F";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B";
            return seq::status::DONE;
        });

    auto g = seq::builder<>{}.add(*s);
    auto seq_impl = builder::build(g);
    REQUIRE(seq_impl.has_value());

    CHECK(seq_impl->forward() == seq::status::DONE);
    CHECK(result == "F");

    CHECK(seq_impl->backward() == seq::status::DONE);
    CHECK(result == "FB");
}

TEST_CASE("build seq with a forward step that takes a while to finish",
          "[seq]") {
    result.clear();
    attempt_count = 0;

    auto s = seq::step<"S">(
        []() -> seq::status {
            if (attempt_count++ < 3) {
                result += "F";
                return seq::status::NOT_DONE;
            } else {
                return seq::status::DONE;
            }
        },
        []() -> seq::status {
            result += "B";
            return seq::status::DONE;
        });

    auto g = seq::builder<>{}.add(*s);
    auto seq_impl = builder::build(g);
    REQUIRE(seq_impl.has_value());

    SECTION("forward can be called") {
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "F");
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "FF");
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "FFF");
        CHECK(seq_impl->forward() == seq::status::DONE);
        CHECK(result == "FFF");
    }
    SECTION(
        "backward can be called, but will not proceed until forward is done") {
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "F");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FF");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FFF");
        CHECK(seq_impl->backward() == seq::status::DONE);
        CHECK(result == "FFFB");
    }
}

TEST_CASE("build seq with a backward step that takes a while to finish",
          "[seq]") {
    result.clear();
    attempt_count = 0;

    auto s = seq::step<"S">(
        []() -> seq::status {
            result += "F";
            return seq::status::DONE;
        },
        []() -> seq::status {
            if (attempt_count++ < 3) {
                result += "B";
                return seq::status::NOT_DONE;
            } else {
                return seq::status::DONE;
            }
        });

    auto g = seq::builder<>{}.add(*s);
    auto seq_impl = builder::build(g);
    REQUIRE(seq_impl.has_value());

    SECTION("backward can be called") {
        CHECK(seq_impl->forward() == seq::status::DONE);
        CHECK(result == "F");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FB");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FBB");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FBBB");
        CHECK(seq_impl->backward() == seq::status::DONE);
        CHECK(result == "FBBB");
    }
    SECTION(
        "forward can be called, but will not proceed until backward is done") {
        CHECK(seq_impl->forward() == seq::status::DONE);
        CHECK(result == "F");
        CHECK(seq_impl->backward() == seq::status::NOT_DONE);
        CHECK(result == "FB");
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "FBB");
        CHECK(seq_impl->forward() == seq::status::NOT_DONE);
        CHECK(result == "FBBB");
        CHECK(seq_impl->forward() == seq::status::DONE);
        CHECK(result == "FBBBF");
    }
}

TEST_CASE("build seq with three steps and run forwards and backwards",
          "[seq]") {
    result.clear();

    auto s1 = seq::step<"S1">(
        []() -> seq::status {
            result += "F1";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B1";
            return seq::status::DONE;
        });

    auto s2 = seq::step<"S2">(
        []() -> seq::status {
            result += "F2";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B2";
            return seq::status::DONE;
        });

    auto s3 = seq::step<"S3">(
        []() -> seq::status {
            result += "F3";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B3";
            return seq::status::DONE;
        });

    auto g = seq::builder<>{}.add(*s1 >> *s2 >> *s3);
    auto seq_impl = builder::build(g);
    REQUIRE(seq_impl.has_value());

    CHECK(seq_impl->forward() == seq::status::DONE);
    CHECK(result == "F1F2F3");
    CHECK(seq_impl->backward() == seq::status::DONE);
    CHECK(result == "F1F2F3B3B2B1");
}
