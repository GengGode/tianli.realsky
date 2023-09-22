namespace tianli.core
{
    public class point
    {
        public int x;
        public int y;
    }

    public class rect
    {
        public int x;
        public int y;
        public int width;
        public int height;

        public rect(int x, int y, int width, int height)
        {
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }

        public rect()
        {
            this.x = 0;
            this.y = 0;
            this.width = 0;
            this.height = 0;
        }

        public bool contains(point p)
        {
            return p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height;
        }

        public bool contains(rect r)
        {
            return r.x >= x && r.x + r.width <= x + width && r.y >= y && r.y + r.height <= y + height;
        }

        public bool intersects(rect r)
        {
            return !(r.x + r.width < x || r.y + r.height < y || r.x > x + width || r.y > y + height);
        }

        public rect intersection(rect r)
        {
            int x1 = Math.Max(x, r.x);
            int x2 = Math.Min(x + width, r.x + r.width);
            int y1 = Math.Max(y, r.y);
            int y2 = Math.Min(y + height, r.y + r.height);
            return new rect(x1, y1, x2 - x1, y2 - y1);
        }

        static public rect operator &(rect r1, rect r2)
        {
            return r1.intersection(r2);
        }

        static public bool operator ==(rect r1, rect r2)
        {
            return r1.x == r2.x && r1.y == r2.y && r1.width == r2.width && r1.height == r2.height;
        }
    }
    public interface IItem
    {
        string Name { get; }
        string Description { get; }
        point Position { get; }
        int index { get; }
    }

    public class ItemObject : IItem
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public point Position { get; set; }
        public int index { get; set; }
    }

    public interface IItemSet
    {
        /// @brief 根据范围查找物品项
        /// @param rect 范围
        /// @return std::vector<std::shared_ptr<ItemInface>> 物品项集合
        List<IItem> find(rect r);
    }

    public class ItemSet : IItemSet
    {
        public class Node
        {
            public enum SplitType
            {
                top_left,
                top_right,
                bottom_left,
                bottom_right
            }
            public rect rect;
            public point center;
            public Node parent;
            public Node top_left;
            public Node top_right;
            public Node bottom_left;
            public Node bottom_right;
            public Node[] childs;
            public List<IItem> items = new List<IItem>();
            public int item_set_size = 0;
            public int node_count = 1; // 至少会有root节点
            public int node_item_max = 128; // 32;// 512;

            Node() { }
            Node(List<IItem> items)
            {
                foreach (var item in items)
                {
                    insert(item);
                }
            }
            Node(Node parent, SplitType splitType)
            {
                this.parent = parent;
                int split_w = parent.rect.width / 2.0;
                int split_h = parent.rect.height / 2.0;
                int split_x = parent.rect.x / 4.0;
                int split_y = parent.rect.y / 4.0;
                switch (splitType)
                {
                    case SplitType.top_left:
                        rect = rect(parent.rect.x, parent.rect.y, split_w, split_h)
                        center = point(parent.rect.x + split_x, parent.rect.y + split_y);
                        break;
                    case SplitType.top_right:
                        rect = rect(parent.rect.x + split_w, parent.rect.y, split_w, split_h)
                        center = point(parent.rect.x + split_x * 3, parent.rect.y + split_y);
                        break;
                    case SplitType.bottom_left:
                        rect = rect(parent.rect.x, parent.rect.y + split_h, split_w, split_h)
                        center = point(parent.rect.x + split_x, parent.rect.y + split_y * 3);
                        break;
                    case SplitType.bottom_right:
                        rect = rect(parent.rect.x + split_w, parent.rect.y + split_h, split_w, split_h)
                        center = point(parent.rect.x + split_x * 3, parent.rect.y + split_y * 3);
                        break;
                }

                for (int i = 0; i < parent.items.Count; i++)
                {
                    if (rect.contains(parent.items[i].Position))
                    {
                        items.Add(parent.items[i]);
                        parent.items.RemoveAt(i);
                    }
                }
                item_set_size = items.Count;
            }
            public bool isLeaf() { return childs == null; }
            public bool isEmtpy() { return items.Count == 0; }
            public bool isIntersect(rect r) { return rect.intersects(r); }
            public bool isIntersect(point p) { return rect.contains(p); }
            public int size() { return items.Count; }
            public int sizes() { return item_set_size; }
            public int count() { return childs.Length; }
            public int counts() { return node_count; }

            public List<Node> spilt()
            {
                if (!isLeaf())
                    return childs;

                top_left = new Node(this, SplitType.top_left);
                top_right = new Node(this, SplitType.top_right);
                bottom_left = new Node(this, SplitType.bottom_left);
                bottom_right = new Node(this, SplitType.bottom_right);
                childs = new Node[] { top_left, top_right, bottom_left, bottom_right };
                if (!items.Count == 0)
                {
                    throw new Exception("spilt error");
                }
                while (this.parent != null)
                {
                    this.parent.item_set_size += 4;
                    this.parent.node_count += 4;
                    this.parent = this.parent.parent;
                }

                return childs;
            }

            public bool insert(IItem item)
            {
                if (!isIntersect(item.Position))
                    return false;

                if (isLeaf())
                {
                    items.Add(item);
                    if (items.Count > node_item_max)
                    {
                        spilt();
                    }
                    return true;
                }

                foreach (var child in childs)
                {
                    if (child.insert(item))
                    {
                        return true;
                    }
                }

                return false;
            }

            public List<IItem> find(rect r)
            {
                List<IItem> result = new List<IItem>();
                if (!isIntersect(r))
                    return result;
                if (isLeaf())
                {
                    if (isIntersect(r.tl()) && isIntersect(r.tr()) && isIntersect(r.bl()) && isIntersect(r.br()))
                    {
                        result.AddRange(items);
                        return result;
                    }

                    foreach (var item in items)
                    {
                        if (r.contains(item.Position))
                        {
                            result.Add(item);
                        }
                    }
                    return result;
                }

                foreach (var child in childs)
                {
                    if (child.isIntersect(r))
                    {
                        result.AddRange(child.find(r));
                    }
                }
                return result;
            }

            public List<Node> findChilds(rect r)
            {
                List<Node> result = new List<Node>();
                if (!isIntersect(r))
                    return result;
                if (isLeaf() && items.Count > 0)
                {
                    result.Add(this);
                    return result;
                }
                foreach (var child in childs)
                {
                    if (child.isIntersect(r))
                    {
                        result.AddRange(child.findChilds(r));
                    }
                }
                return result;
            }
        }

        Node root;
        public ItemSet(rect rect, List<IItem> items)
        {
            root = new Node();
            root.rect = rect;
            root.center = point(rect.x + rect.width / 2, rect.y + rect.height / 2);
            foreach (var item in items)
            {
                root.insert(item);
            }
        }

        public List<IItem> find(rect r) override
        {
            return root.find(r);
        }

    public List<Node> findChilds(rect r)
    {
        return root.findChilds(r);
    }
}
}