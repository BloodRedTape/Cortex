#include <iostream>
#include "repository.hpp"

int main()
{
	Repository repo;
	repo.OnDirChanged({FileActionType::Create, {}, "file1.txt"});
	repo.OnDirChanged({FileActionType::Create, {}, "file2.txt"});
	repo.OnDirChanged({FileActionType::Create, {}, "file3.txt"});
	repo.OnDirChanged({FileActionType::Update, {}, "file1.txt"});
	repo.OnDirChanged({FileActionType::Delete, {}, "file2.txt"});
	return 0;
}
