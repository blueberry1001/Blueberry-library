using namespace std;
#include <bits/stdc++.h>
struct Graph
{
	vector<vector<pair<int, long long>>> g;
	int V{};
	bool weighted = false;
	vector<long long> dist;
	Graph(int n) : g(n)
	{
		V = n;
	}
	void add_edge(int u, int v, long long w = 1, int dir = 0)
	{
		assert(u < V && v < V);
		if (w != 1)weighted = true;
		g[u].push_back({v, w});
		if (!dir)g[v].push_back({u, w});
	}
	void init(int m, int mode = 0, int dir = 0)
	{
		for (int i = 0; i < m; i++)
		{
			int u, v;
			cin >> u >> v;
			u--,v--;
			long long w = 1;
			if (mode)cin >> w;
			add_edge(u, v, w, dir);
		}
	}
	long long distance(int start, int end)
	{
		dist.assign(V, ((1LL<<62)-(1LL<<31)));
		dist[start]=0;
		if (weighted)
		{
			priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<pair<long long, int>>> pq;
			pq.push({0, start});
			while (pq.size())
			{
				auto [di, pos] = pq.top();pq.pop();
				if (dist[pos] < di)continue;
				for (auto [to, d] : g[pos])
				{
					if (dist[to] > dist[pos] + d)
					{
						dist[to] = di + d;
						pq.push({dist[to], to});
					}
				}
			}
		}
		else
		{
			queue<int> q;
			q.push(start);
			while (q.size())
			{
				int pos = q.front();q.pop();
				for (auto [to, d] : g[pos])
				{
					if (dist[to] > dist[pos] + 1)
					{
						dist[to] = dist[pos] + 1;
						q.push(to);
					}
				}
			}
		}
		return dist[end];
	}
};