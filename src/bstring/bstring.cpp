#include "binarystring.hpp"

bstring::bstring() : __string(NULL), stringsize(0) {}

bstring::bstring(const char* str, const size_t size) : stringsize(size) {
	if (str == NULL) {
		__string = NULL;
		stringsize = 0;
	} else {
		__string = new char[stringsize];
		for (size_t i = 0; i < stringsize; ++i) {
			__string[i] = str[i];
		}
	}
}

bstring::bstring(const bstring& other) {
	// cerr << "bstring::copy constructer called" << endl;
	if (other.empty() == false) {
		stringsize = other.size();
		__string = new char[stringsize];
		for (size_t i = 0; i < stringsize; ++i) {
			__string[i] = other.__string[i];
		}
	} else {
		stringsize = 0;
		__string = NULL;
	}
}

bstring::~bstring() {
	// cerr << "bstring::destructor called" << endl;
	if (__string)
		delete []__string;
}

const size_t&	bstring::size() const {
	return stringsize;
}

const string	bstring::cppstring() const {
	std::string s;
	for (size_t i = 0; i < stringsize; ++i)
		s += __string[i];
	return s;
}

const char*	bstring::c_str() const {//add \0 in the end
	return __string;
}

vector<bstring>	bstring::split(const char* seperator) const {
	vector<bstring> list;
	size_t			i = 0, pos = 0;
	while(i < stringsize) {
		if (ncmp(seperator, strlen(seperator), i) == 0) {
			list.push_back(substr(pos, i-pos));
			i += strlen(seperator);
			while (ncmp(seperator, strlen(seperator), i) == 0)
				i += strlen(seperator);
			pos = i;
		} else
			++i;
	}
	if (pos < stringsize)
		list.push_back(substr(pos));
	return list;
}

bstring	bstring::substr(size_t start, size_t len) const {
	size_t	i = 0;

	if (__string == NULL || start >= stringsize)
		return bstring();//default constructer just like returning NULL;
	if (len > stringsize - start || len == std::string::npos)
		len = stringsize - start;
	char substring[len];
	while (i < len && i < stringsize) {
		substring[i] = __string[start + i];
		i++;
	}
	return bstring(substring, len);
}

bool	bstring::getline(bstring& line) {
	char	ch;
	for (size_t i = 0; i < stringsize; ++i) {
		ch = __string[i];
		if (ch == '\n') {
			line = substr(0, i+1);
			erase(0, i+1);
			return true;
		}
	}
	line = substr(0);
	erase(0, std::string::npos);
	return false;
}

void	bstring::erase(const size_t start, size_t n) {
	if (start >= stringsize)
		return ;
	else if (n >= stringsize) {
		n = stringsize - start;
	}
	if (stringsize - n == 0) {
		delete []__string;
		__string = NULL;
		stringsize = 0;
		return ;
	}
	char* newstring = new char[stringsize-n];
	size_t i = 0, newstringindex = 0;
	while (i < start) {
		newstring[newstringindex] = __string[i];
		++i;
		++newstringindex;
	}
	i = start + n;
	while (i < stringsize) {
		newstring[newstringindex] = __string[i];
		++i;
		++newstringindex;
	}
	delete[] __string;
	__string = newstring;
	stringsize = stringsize - n;
}

size_t	bstring::find(const char* needle, const size_t startpos) const {
	if (startpos >= stringsize)
		return std::string::npos;
	for (size_t i = startpos; i < stringsize; ++i) {
		if (ncmp(needle, strlen(needle), i) == 0)
			return i;
	}
	return std::string::npos;
}

size_t	bstring::find(const char needle, const size_t startpos) const {
	if (startpos >= stringsize)
		return std::string::npos;
	for (size_t i = startpos; i < stringsize; ++i) {
		if (__string[i] == needle)
			return i;
	}
	return std::string::npos;
}

size_t	bstring::rfind(const char needle, size_t startpos) const {
	if (startpos > stringsize-1)
		startpos = stringsize - 1;
	for (ssize_t i = startpos; i >= 0; --i) {
		if (__string[i] == needle)
			return i;
	}
	return std::string::npos;
}

bstring bstring::trimend(const char* chars) const{
	ssize_t i = stringsize - 1;
	bstring bstringchars(chars, strlen(chars));
	bstring	objectcopy(*this);

	while (i >= 0) {
		if (bstringchars.find(__string[i]) != std::string::npos)
			--i;
		else
			break;
	}
	objectcopy.erase(i+1, std::string::npos);
	return objectcopy;
}

bool	bstring::cmp(const char* str) const {
	size_t	i = 0;
	if (strlen(str) != stringsize)
		return true;
	while(i+1 < stringsize && str[i] && __string[i] == str[i])
		++i;
	return __string[i] - str[i];
}

bool	bstring::ncmp(const char* str1, const size_t n, const size_t startpos) const {
	int i = 0;

	if (startpos >= stringsize)
		return true;
	const char* str2 = &(__string[startpos]);
	size_t str2size = stringsize - startpos;
	if (n > str2size)
		return true;
	while (i+1 < n && i+1 < str2size && str1[i] && str2[i] == str1[i])
		++i;
	return str1[i] - str2[i];
}

// bool	bstring::ncmp(const bstring& str1, const size_t n, const size_t startpos) const {
// 	int i = 0;

// 	if (startpos >= stringsize)
// 		return true;
// 	const char* str2 = &(__string[startpos]);
// 	size_t str2size = stringsize - startpos;
// 	if (n > str2size)
// 		return true;
// 	while (i < n && i+1 < str2size && i+1 < str1.size() && str2[i] == str1[i])
// 		++i;
// 	return str1[i] - str2[i];
// }


bool	bstring::empty() const {
	if (__string == NULL)
		return true;
	return false;
}

const bstring& bstring::operator=(const bstring& other) {
	if (this != &other) {
		delete[] __string;
		stringsize = other.size();
		if (other.empty())
			__string = NULL;
		else {
			__string = new char[stringsize];
			for (size_t i = 0; i < stringsize; ++i) {
				__string[i] = other.__string[i];
			}
		}
	}
	return *this;
}

const char*	bstring::operator=(const char* newstring) {
	delete []__string;
	if (newstring == NULL) {
		stringsize = 0;
		__string = NULL;
		return NULL;
	}
	stringsize = strlen(newstring);
	__string = new char[stringsize];
	for (size_t i = 0; i < stringsize; ++i) {
		__string[i] = newstring[i];
	}
	return __string;
}

const char&	bstring::operator[](const int index) const{
	if (index < 0 || index >= stringsize)
		throw(bstringExceptions("index out of range"));
	return __string[index];
}

std::ostream& operator<<(std::ostream &out, const bstring &bs) {
	for (size_t i = 0; i < bs.size(); ++i)
		out << bs[i];
	return out;
}

const bstring&	bstring::operator+=(const bstring& buff) {
	if (buff.empty())
		return *this;
	char* newstring = new char[stringsize+buff.size()];
	int	newstringpos = 0;
	for (size_t i = 0; i < stringsize; ++i) {
		newstring[newstringpos] = __string[i];
		++newstringpos;
	}
	for (size_t i = 0; i < buff.size(); ++i) {
		newstring[newstringpos] = buff[i];
		++newstringpos;
	}
	delete[] __string;
	__string = newstring;
	stringsize += buff.size();
	return *this;
}

const char*	bstring::operator+=(const char* buff) {
	if (buff == NULL)
		return __string;
	char* newstring = new char[stringsize+strlen(buff)];
	int	newstringpos = 0;
	for (size_t i = 0; i < stringsize; ++i) {
		newstring[newstringpos] = __string[i];
		++newstringpos;
	}
	for (size_t i = 0; i < strlen(buff); ++i) {
		newstring[newstringpos] = buff[i];
		++newstringpos;
	}
	delete[] __string;
	__string = newstring;
	stringsize += strlen(buff);
	return __string;
}

const bstring	bstring::operator+(const bstring& buff) {
	if (buff.empty())
		return *this;
	char* newstring = new char[stringsize+buff.size()];
	int	newstringpos = 0;
	for (size_t i = 0; i < stringsize; ++i) {
		newstring[newstringpos] = __string[i];
		++newstringpos;
	}
	for (size_t i = 0; i < buff.size(); ++i) {
		newstring[newstringpos] = buff[i];
		++newstringpos;
	}
	return bstring(newstring, stringsize+buff.size());
}