using Airlanes.DataSetAirlanesTableAdapters;
using System;
using System.Data;
using System.Windows;

namespace Airlanes
{
    /// <summary>
    /// Логика взаимодействия для Administrator.xaml
    /// </summary>
    public partial class Administrator : Window
    {
        UserViewTableAdapter userViewTableAdapter = new UserViewTableAdapter();
        OfficesTableAdapter officesTableAdapter = new OfficesTableAdapter();
        UsersTableAdapter usersTableAdapter = new UsersTableAdapter();
        public Administrator()
        {
            InitializeComponent();
            dataUsers.ItemsSource = userViewTableAdapter.GetData();
            offices.ItemsSource = officesTableAdapter.GetData();
        }

        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            MainWindow main = new MainWindow();
            Close();
            main.Show();
        }

        private void enable_disable_Click(object sender, RoutedEventArgs e)
        {
            DataRowView drv = (DataRowView)dataUsers.SelectedItem;
            String active = drv["Active"].ToString();
            if (drv != null)
            {
                try
                {
                    if (Convert.ToBoolean(active) == true)
                    {
                        string id = drv["ID"].ToString();
                        usersTableAdapter.UpdateActive(false, Convert.ToInt32(id));
                        dataUsers.ItemsSource = userViewTableAdapter.GetData();
                    }
                    else
                    {
                        string id = drv["ID"].ToString();
                        usersTableAdapter.UpdateActive(true, Convert.ToInt32(id));
                        dataUsers.ItemsSource = userViewTableAdapter.GetData();
                    }
                }
                catch
                {
                    MessageBox.Show("Не удалось сохранить изменения");
                }
            }
            else
            {
                MessageBox.Show("Вы не выбрали пользователя");
            }
        }

        private void offices_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if(offices.SelectedIndex != -1)
            {
                dataUsers.ItemsSource = userViewTableAdapter.GetDataBy(offices.SelectedValue.ToString()); 
            }
        }

        private void reset_Click(object sender, RoutedEventArgs e)
        {
            offices.SelectedIndex = -1;
            dataUsers.ItemsSource = userViewTableAdapter.GetData();
        }

        private void btnAddUser_Click(object sender, RoutedEventArgs e)
        {
            AddUser addUser = new AddUser(this);
            IsEnabled = false;
            addUser.Show();
        }

        private void delete_Click(object sender, RoutedEventArgs e)
        {
            DataRowView drv = (DataRowView)dataUsers.SelectedItem;          
            if (drv != null)
            {
                String id = drv["ID"].ToString();
                usersTableAdapter.DeleteQuery(Convert.ToInt32(id));
                dataUsers.ItemsSource = userViewTableAdapter.GetData();
            }
            else
            {
                MessageBox.Show("Вы не выбрали пользователя для удаления");
            }
        }

        private void changeRole_Click(object sender, RoutedEventArgs e)
        {
            DataRowView drv = (DataRowView)dataUsers.SelectedItem;
            if (drv != null)
            {
                String id = drv["ID"].ToString();
                EditUser editUser = new EditUser(this, id);
                editUser.Show();
            }
            else
            {
                MessageBox.Show("Вы не выбрали пользователя для редактирования");
            }
        }
    }
}
