// C++ demo of Resource Acquisition Is initialization without exceptions
#include <variant>
#include <iostream>

using namespace std;

/// Rust-like Result, basically a wrapper over std::variant
template<class Ok, class Err>
struct Result {
private:
	std::variant<Ok, Err> m;
public:
	Result() = delete;
	Result(const Result &) = default;
	Result &operator=(const Result &) & = default;

	Result(Ok ok)
		: m(std::move(ok)) {}
	Result(Err err)
		: m(std::move(err)) {}
	Ok *ok() {
		return std::get_if<Ok>(&m);
	}
	Ok unwrap() {
		return std::move(std::get<Ok>(m));
	}
	Err *err() {
		return std::get_if<Err>(&m);
	}
};

/// A file that will fail to "open" if path is empty
class DemoFile {
private:
    char *path;
    DemoFile(char *path) : path(path) {}
    int initialize() {
        if (path == nullptr) {
            return -1;
        }
        cout << "opened";
        return 0; // successfully initialized
    }
public:
    ~DemoFile() {
        cout << "closed";
    }

    static Result<DemoFile, int> open(char *path) {
        auto d = DemoFile(path);
        int ret = d.initialize();
        if (ret != 0) {
            return ret; // fail by returning int (error code)
        }
        return d; // succeed by returning object
    }
    void write(char *data) {
        // benefit #1: there's no need to check validity in methods.
        // if the object exists, it's valid and its methods may be used,
        // no way to call the "wrong" method
        cout << "written";
    }
};

void open_or_throw(void) {
    auto outcome = DemoFile::open("foo");
    DemoFile file = outcome.unwrap(); // will throw on failure. simplest, easiest.
    file.write("xx");
}

void open_oops_throw(void) {
    auto outcome = DemoFile::open(nullptr);
    DemoFile file = outcome.unwrap();
    // this code can never run, invalid DemoFile may not be issued
    cout << "successfully obtained invalid object";
}

void open_or_report_basic(void) {
    auto outcome = DemoFile::open("foo");
    // have we got success?
    DemoFile *file = outcome.ok();
    if (file != nullptr) {
        file->write("xx");
        // file is a fully initialized object
    } else {
        int *errcode = outcome.err();
        // file is nullptr,
        // errcode can't be nullptr
        cout << "failed with" << *errcode;
    }
}

// a bit less verbose version of the above
void open_or_report_realistic(void) {
    auto outcome = DemoFile::open("foo");
    // have we got success?
    if (outcome.err() != nullptr) {
        int *err = outcome.err();
        cout << "failed with" << *err;
        return;
    }
    // err() was nullptr, so ok() will be the initialized object,
    // no danger of exceptions.
    DemoFile file = outcome.unwrap();
    file.write("");
    
    // Benefit #2:
    // The Demofile either doesn't exist or is working,
    // the user code can't get a half-initialized one.
    // If it can be declared, it's valid.
    
    // This means that the API user doesn't need to ask "is this class initialized correctly?", but can instead ask "is this pointer initialized?", which is simpler and uniform across objects.
    // This benefit spreads throughout the code base:
    // with diligent application, there are no invalid objects that outlive the function scope,
    // and the programmer looking through headers doesn't have to wonder about the failure modes of class members relating to their state: if it can be declared as a member, it's valid.
}

int main(void) {

	open_or_report_basic();

	cerr << endl;
	cout << endl;

	open_oops_throw();

	cerr << endl;
	cout << endl;
}

